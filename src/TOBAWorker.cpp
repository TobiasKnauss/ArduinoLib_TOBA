#include <WOCO_AliveCheck.h>
#include <WOCO_DeviceName.h>
#include <WOCO_DeviceType.h>

#include "TOBAWorker.h"
#include "TOBAWorkerConfig.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
::EResult TOBAWorker::Create (Stream*           i_pCommStream,
                              UCOP*             i_pUCOP,
                              TOBAWorkerConfig* i_pConfig,
                              TOBAWorker*&      o_pWorker)
{
  o_pWorker = nullptr;

  TOBADevice* pDevice = nullptr;
  ::EResult result = TOBADevice::Create (i_pCommStream, i_pUCOP, i_pConfig, pDevice);
  if (result != ::EResult::SUCCESS)
    return result;

  o_pWorker = (TOBAWorker*)pDevice;
  return result;
}

//--------------------------------------------------------------------
TOBAWorker::TOBAWorker ()
{
}

//--------------------------------------------------------------------
TOBAWorker::~TOBAWorker ()
{
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
::EResult TOBAWorker::AnalyzeData ()
{
  if (get_IsBusy ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsBusy;

  if (!m_DataAvailable)
    return (::EResult)TOBA::EResult::FAIL_TOBA_DataMissing;

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
    return (::EResult)TOBA::EResult::FAIL_TOBA_RequestMissing;
  if (!m_ReplyData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_pWOCO != nullptr)
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsWorking;

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
  TOBADevice::Clear ();
  ClearReqRepWoco ();
}

//--------------------------------------------------------------------
void TOBAWorker::ClearReqRepWoco ()
{
  m_ReplyData  .Clear ();
  m_RequestData.Clear ();
  DeleteObject (m_pWOCO);
}

//--------------------------------------------------------------------
::EResult TOBAWorker::SendReply ()
{
  if (m_pWOCO != nullptr)
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsWorking;
  if (!m_RequestData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_ReplyData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_ReplyMissing;

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
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_pWOCO == nullptr)
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerCommandMissing;

  WOCO* pWORE = nullptr;  // WOrker REply
  UCOP::EMessageResult messageResult = UCOP::EMessageResult::None;
  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::DeviceType:
    pWORE = WOCO_DeviceType::CreateReadReply ((uint32_t)get_DeviceType ());
    messageResult = UCOP::EMessageResult::SUCCESS;
    break;

  case WOCO::ECommand::DeviceName:
    pWORE = WOCO_DeviceName::CreateReadReply (get_DeviceName (), get_DeviceNameLength ());
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
::EResult TOBAWorker::Init (Stream*           i_pCommStream,
                            UCOP*             i_pUCOP,
                            TOBADeviceConfig* i_pConfig)
{
  ::EResult result = TOBADevice::Init (i_pCommStream, i_pUCOP, i_pConfig);
  if (result != ::EResult::SUCCESS)
    return result;

  m_pConfig = (TOBAWorkerConfig*)i_pConfig;

  return ::EResult::SUCCESS;
}
