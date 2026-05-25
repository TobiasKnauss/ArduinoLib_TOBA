#include <WOCO_AliveCheck.h>
#include <WOCO_WorkerName.h>
#include <WOCO_WorkerType.h>

#include "TOBAWorker.h"
#include "TOBAConfig.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
#define X(name) const char TOBAWorker::_EResult_##name[] PROGMEM = #name;
#include "TOBA_EResult_failures.h"
#undef X

//--------------------------------------------------------------------
#define X(name) _EResult_##name,
const char* const TOBAWorker::c_EResult_ClassFailures_Names[] PROGMEM =
{
  #include "TOBA_EResult_failures.h"
};
#undef X

//--------------------------------------------------------------------
::EResult TOBAWorker::Create (Stream*       i_pCommStream,
                              UCOP*         i_pUCOP,
                              TOBAConfig*   i_pConfig,
                              TOBAWorker*&  o_pWorker)
{
  o_pWorker = nullptr;
  TOBAWorker* pWorker = new TOBAWorker (i_pCommStream,
                                        i_pUCOP,
                                        i_pConfig);

  ::EResult result = pWorker->Verify_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete pWorker;
    return result;
  }

  result = pWorker->CreateDevices_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete pWorker;
    return result;
  }

  o_pWorker = pWorker;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBAWorker::~TOBAWorker ()
{
  Clear ();

  if (m_NeedToDeleteUCOP)
  {
    DeleteObject (m_pUCOP);
    DeleteObject (m_pUCOPConfig);
  }

  DeleteObject (m_pReceiveBuffer);
  DeleteObject (m_pSendBuffer);
  DeleteObject (m_pPayloadRecvBuffer);
  DeleteObject (m_pPayloadSendBuffer);
}

//--------------------------------------------------------------------
TOBAWorker::TOBAWorker (Stream*     i_pCommStream,
                        UCOP*       i_pUCOP,
                        TOBAConfig* i_pConfig)
{
  m_pCommStream = i_pCommStream;
  m_pUCOP       = i_pUCOP;
  m_pConfig     = i_pConfig;
}

//--------------------------------------------------------------------
bool TOBAWorker::get_ExistsData ()
{
  return m_DataAvailable;
}

//--------------------------------------------------------------------
bool TOBAWorker::get_ExistsReply ()
{
  return !m_ReplyData.IsEmpty ();
}

//--------------------------------------------------------------------
bool TOBAWorker::get_ExistsRequest ()
{
  return !m_RequestData.IsEmpty ();
}

//--------------------------------------------------------------------
bool TOBAWorker::get_ExistsWork ()
{
  return m_pWOCO != nullptr;
}

//--------------------------------------------------------------------
bool TOBAWorker::get_IsBusy ()
{
  return get_ExistsRequest ()
      || get_ExistsReply ()
      || get_ExistsWork();
}

//--------------------------------------------------------------------
bool TOBAWorker::get_IsWorking ()
{
  return m_pWOCO != nullptr;
}

//--------------------------------------------------------------------
char* TOBAWorker::get_WorkerName ()
{
  return m_pConfig->get_WorkerName ();
}

//--------------------------------------------------------------------
uint8_t TOBAWorker::get_WorkerNameLength ()
{
  return m_pConfig->get_WorkerNameLength ();
}

//--------------------------------------------------------------------
TOBAWorker::EWorkerType TOBAWorker::get_WorkerType ()
{
  return EWorkerType::BuiltIn;
}

//--------------------------------------------------------------------
const __FlashStringHelper* TOBAWorker::GetResultText (::EResult i_Result)
{
  if ((uint16_t)i_Result < (uint16_t)EResult::Dummy_FirstClassFailure)
    return Result::GetText (i_Result);
  return (const __FlashStringHelper*)pgm_read_ptr(&c_EResult_ClassFailures_Names[(uint16_t)i_Result - (uint16_t)EResult::Dummy_FirstClassFailure - 1]);
}

//--------------------------------------------------------------------
::EResult TOBAWorker::AnalyzeData ()
{
  if (get_IsBusy ())
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;

  if (!m_DataAvailable)
    return (::EResult)EResult::FAIL_TOBA_DataMissing;

  ::EResult result = ::EResult::InProgress;
  bool     receivedMessageTypeIsReply = false;
  uint16_t receivedMessageLength      = 0;
  UCOPData receivedData;
  receivedData.SetPayloadInfo (m_pPayloadRecvBuffer,
                               m_PayloadBuffersSize);

  // Analyze message in the received data
  uint16_t receiveBufferReadIndexAtAnalyzeStart = m_ReceiveBufferReadIndex;
  result = m_pUCOP->SearchMessage (m_pReceiveBuffer,
                                   m_pConfig->get_ReceiveBufferSize (),
                                   m_ReceiveBufferReadIndex,
                                   receivedData,
                                   receivedMessageTypeIsReply,
                                   receivedMessageLength);

  // In the ring buffer, clear the part that has been analysed (and maybe contains a message).
  RingBuffer_SetValue_StartToEnd (m_pReceiveBuffer,
                                  m_pConfig->get_ReceiveBufferSize (),
                                  receiveBufferReadIndexAtAnalyzeStart,
                                  m_ReceiveBufferReadIndex,
                                  c_BufferDefaultValue);

  #ifdef TOBA_DEBUG
  Serial << F("UCOP.SearchMessage() result=") << UCOP::GetResultText (result) << endl;

  Serial << F("Message Type is Reply: ") << receivedMessageTypeIsReply          << endl;
  Serial << F("Action is Write:       ") << receivedData.ActionIsWrite          << endl;
  Serial << F("Remote Device Id:      ") << _HEX8 (receivedData.RemoteDeviceId) << endl;
  Serial << F("Message Id:            ") << receivedData.MessageId              << endl;
  Serial << F("Timestamp:             ") << receivedData.Timestamp              << endl;
  Serial << F("CommandId:             ") << receivedData.CommandId              << endl;
  Serial << F("Result:                ") << (uint8_t)receivedData.MessageResult << endl;
  Serial << F("Payload Data Length:   ") << receivedData.PayloadLength          << endl;
  Serial << F("Message Length:        ") << receivedMessageLength               << endl;

  Serial << F("ReceiveBuffer:") << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_pConfig->get_ReceiveBufferSize ());
  Serial << F("PayloadRecvBuffer: bytes used = ") << receivedData.PayloadLength << endl;
  Memory_PrintLn (m_pPayloadRecvBuffer, m_PayloadBuffersSize);
  #endif

  if (receivedMessageTypeIsReply)  // Flag is only set if a message was found.
  {
    #ifdef TOBA_DEBUG
    Serial << F("Message is no request. Nothing to do.");
    #endif
    // Sending a reply ourselves is useless since no reply is expected.
    // Sending a reply must furthermore be avoided because it would start reply ping-pong.
    return ::EResult::SUCCESS;
  }

  UCOP::EMessageResult messageResult = UCOP::GetMessageResultForFunctionResult (result);
  if (messageResult != UCOP::EMessageResult::None)
  {
    // Message is faulty. Do not process it, but send a failure reply.
    #ifdef TOBA_DEBUG
    Serial << F("Message is faulty. Creating failure reply.");
    #endif
    m_ReplyData = UCOPData::CreateReplyData (receivedData, GetTimestamp (), messageResult);
  }

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_NotFound)
  {
    memset (m_pReceiveBuffer, c_BufferDefaultValue, m_pConfig->get_ReceiveBufferSize ());
    m_DataAvailable = false;
    return ::EResult::SUCCESS;
  }

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_ReceiverDeviceIdMismatch)  // Message is not sent to this device. Nothing to do.
    return ::EResult::SUCCESS;

  #ifdef TOBA_DEBUG
  Serial << F("Message is a request.");
  #endif
  m_RequestData = receivedData;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::AnalyzeRequest ()
{
  if (m_RequestData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_RequestMissing;
  if (!m_ReplyData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_pWOCO != nullptr)
    return (::EResult)EResult::FAIL_TOBA_WorkerIsWorking;

  ::EResult result;
  WOCO* pWOCO = nullptr;

  result = WOCO::Create ((WOCO::ECommand)m_RequestData.CommandId,
                         WOCO::TYPE_Request,
                         m_RequestData.ActionIsWrite,
                         pWOCO);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.Create() result=") << WOCO::GetResultText (result) << endl;
  #endif

  if (result != ::EResult::SUCCESS)
  {
    m_ReplyData = UCOPData::CreateReplyData (m_RequestData, GetTimestamp (), UCOP::EMessageResult::FAIL_CommandNotSupported);

    DeleteObject (pWOCO);
    m_RequestData.Clear ();
    return result;
  }

  result = m_pWOCO->AnalyzeCommandData (m_RequestData.pPayloadBuffer,
                                        m_RequestData.PayloadBufferLength,
                                        m_RequestData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.AnalyzeCommandData() result=") << WOCO::GetResultText (result) << endl;
  #endif

  memset (m_RequestData.pPayloadBuffer, c_BufferDefaultValue, m_RequestData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("PayloadRecvBuffer:") << endl;
  Memory_PrintLn (m_pPayloadRecvBuffer, m_PayloadBuffersSize);
  #endif

  if (result != ::EResult::SUCCESS)
  {
    m_ReplyData = UCOPData::CreateReplyData (m_RequestData, GetTimestamp (), UCOP::EMessageResult::FAIL_PayloadProcessing);

    DeleteObject (pWOCO);
    m_RequestData.Clear ();
    return result;
  }

  #ifdef TOBA_DEBUG
  Serial << F("Command is valid. Start working...");
  #endif
  m_pWOCO = pWOCO;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
void TOBAWorker::Clear ()
{
  ClearBuffers ();
  ClearReqRepWoco ();
}

//--------------------------------------------------------------------
void TOBAWorker::ClearBuffers ()
{
  memset (m_pReceiveBuffer    , c_BufferDefaultValue, m_pConfig->get_ReceiveBufferSize () );
  memset (m_pSendBuffer       , c_BufferDefaultValue, m_pConfig->get_SendBufferSize ()    );
  memset (m_pPayloadRecvBuffer, c_BufferDefaultValue, m_pConfig->get_PayloadBuffersSize ());
  memset (m_pPayloadSendBuffer, c_BufferDefaultValue, m_pConfig->get_PayloadBuffersSize ());
}

//--------------------------------------------------------------------
void TOBAWorker::ClearReqRepWoco ()
{
  m_ReplyData  .Clear ();
  m_RequestData.Clear ();
  DeleteObject (m_pWOCO);
}

//--------------------------------------------------------------------
uint32_t TOBAWorker::GetTimestamp ()
{
  return 0;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::ReadData ()
{
  if (m_pCommStream == nullptr)
    return (::EResult)EResult::FAIL_TOBA_CommStream_NotReady;
  if (!m_pCommStream->available ())
    return ::EResult::SUCCESS;

  #ifdef TOBA_DEBUG
  Serial << F("CommStream available: ") << m_pCommStream->available () << endl;
  #endif

  // Receive all available data
  while (m_pCommStream->available ())
  {
    m_pReceiveBuffer[m_ReceiveBufferWriteIndex++] = m_pCommStream->read ();
    if (m_ReceiveBufferWriteIndex >= m_pConfig->get_ReceiveBufferSize ())
      m_ReceiveBufferWriteIndex = 0;
  }

  #ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer: position = ") << m_ReceiveBufferWriteIndex << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_pConfig->get_ReceiveBufferSize ());
  #endif

  m_DataAvailable = true;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::SendReply ()
{
  if (m_pWOCO != nullptr)
    return (::EResult)EResult::FAIL_TOBA_WorkerIsWorking;
  if (!m_RequestData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_ReplyData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_ReplyMissing;

  uint16_t replyMessageLength = 0;
  ::EResult result = m_pUCOP->ComposeReply (m_ReplyData,
                                            m_pSendBuffer,
                                            m_pConfig->get_SendBufferSize (),
                                            replyMessageLength);

  m_ReplyData.Clear ();
  memset (m_pPayloadSendBuffer, c_BufferDefaultValue, m_ReplyData.PayloadLength);

  #ifdef TOBA_DEBUG
  Serial << F("UCOP.ComposeReply() result=") << UCOP::GetResultText (result) << endl;
  Serial << F("SendBuffer: bytes used = ") << replyMessageLength << endl;
  Memory_PrintLn (m_pSendBuffer, m_pConfig->get_SendBufferSize ());
  #endif

  if (result == ::EResult::SUCCESS)
  {
    m_pCommStream->write (m_pSendBuffer, replyMessageLength);
    m_pCommStream->flush ();
  }

  memset (m_pSendBuffer, c_BufferDefaultValue, replyMessageLength);

  return result;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::Work ()
{
  if (!m_ReplyData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_pWOCO == nullptr)
    return (::EResult)EResult::FAIL_TOBA_WorkerCommandMissing;

  WOCO* pWORE = nullptr;  // WOrker REply
  UCOP::EMessageResult messageResult = UCOP::EMessageResult::None;
  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::WorkerType:
    pWORE = WOCO_WorkerType::CreateReadReply ((uint32_t)get_WorkerType ());
    messageResult = UCOP::EMessageResult::SUCCESS;
    break;

  case WOCO::ECommand::WorkerName:
    pWORE = WOCO_WorkerName::CreateReadReply (get_WorkerName (), get_WorkerNameLength ());
    messageResult = UCOP::EMessageResult::SUCCESS;
    break;

  case WOCO::ECommand::AliveCheck:
    pWORE = WOCO_AliveCheck::CreateReadReply ();
    messageResult = UCOP::EMessageResult::SUCCESS;
    break;

  default:
    return ::EResult::InProgress;
  }

  m_ReplyData = UCOPData::CreateReplyData (m_RequestData, GetTimestamp (), messageResult);
  m_ReplyData.SetPayloadInfo (m_pPayloadSendBuffer, m_PayloadBuffersSize);

  ::EResult result = pWORE->ComposeCommandData (m_ReplyData.pPayloadBuffer,
                                                m_ReplyData.PayloadBufferLength,
                                                m_ReplyData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.ComposeCommandData() result=") << UCOP::GetResultText (result) << endl;
  #endif

  m_RequestData.Clear ();
  DeleteObject (m_pWOCO);
  DeleteObject (pWORE);

  return result;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::CreateDevices_EXEC ()
{
  ::EResult result;
  if (m_pUCOP == nullptr)
  {
    UCOPConfig* pUCOPConfig = nullptr;
    result = UCOPConfig::Create (m_pConfig->get_EepromAddress_UCOPConfig (), pUCOPConfig);
    #ifdef TOBA_DEBUG
    Serial << F("UCOPConfig.Create() result=") << UCOP::GetResultText (result) << endl;
    #endif
    if (result != ::EResult::SUCCESS)
      return result;

    UCOP* pUCOP = nullptr;
    result = UCOP::Create (pUCOPConfig, pUCOP);;
    #ifdef TOBA_DEBUG
    Serial << F("UCOP.Create() result=") << UCOP::GetResultText (result) << endl;
    #endif
    if (result != ::EResult::SUCCESS)
    {
      delete pUCOPConfig;
      return result;
    }

    m_NeedToDeleteUCOP = true;
    m_pUCOPConfig = pUCOPConfig;
    m_pUCOP = pUCOP;
  }

  if (!Memory_Allocate (m_pReceiveBuffer    , m_pConfig->get_ReceiveBufferSize () , c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pSendBuffer       , m_pConfig->get_SendBufferSize ()    , c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pPayloadRecvBuffer, m_pConfig->get_PayloadBuffersSize (), c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pPayloadSendBuffer, m_pConfig->get_PayloadBuffersSize (), c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;

  #ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer     Addr=") << _HEX4((uint16_t)m_pReceiveBuffer)     << F(" Len=") << m_pConfig->get_ReceiveBufferSize () << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_pConfig->get_ReceiveBufferSize ());
  Serial << F("SendBuffer        Addr=") << _HEX4((uint16_t)m_pSendBuffer)        << F(" Len=") << m_pConfig->get_SendBufferSize () << endl;
  Memory_PrintLn (m_pSendBuffer, m_pConfig->get_SendBufferSize ());
  Serial << F("PayloadRecvBuffer Addr=") << _HEX4((uint16_t)m_pPayloadRecvBuffer) << F(" Len=") << m_pConfig->get_PayloadBuffersSize () << endl;
  Memory_PrintLn (m_pPayloadRecvBuffer, m_pConfig->get_PayloadBuffersSize ());
  Serial << F("PayloadSendBuffer Addr=") << _HEX4((uint16_t)m_pPayloadSendBuffer) << F(" Len=") << m_pConfig->get_PayloadBuffersSize () << endl;
  Memory_PrintLn (m_pPayloadSendBuffer, m_pConfig->get_PayloadBuffersSize ());
  #endif

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::Verify_EXEC ()
{
  if (m_pCommStream == nullptr
  ||  m_pConfig     == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  if (m_pConfig->get_WorkerType () != get_WorkerType ())
    return ::EResult::FAIL_Device_ConfigTypeWrong;

  return ::EResult::SUCCESS;
}
