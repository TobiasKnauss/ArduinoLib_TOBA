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

  return ::EResult::SUCCESS;
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
TOBAWorkerConfig* TOBAWorker::get_Config ()
{
  return m_pConfig;
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
  receivedData.SetPayloadInfo (m_pPayloadRecvBuffer);

  // Analyze message in the received data
  result = m_pUCOP->SearchMessage (m_pReceiveBuffer,
                                   receivedData,
                                   receivedMessageTypeIsReply,
                                   receivedMessageLength);

  if (result == ::EResult::SUCCESS)
  {
    // In the ring buffer, clear the part that has been analysed (and maybe contains a message).
    m_pReceiveBuffer->Clear_To (m_pReceiveBuffer->get_CurrentReadAddress (),
                                receivedMessageLength);
  }

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
  m_pReceiveBuffer->Print (&Serial, true);
  Serial << F("PayloadRecvBuffer: bytes used = ") << receivedData.PayloadLength << endl;
  m_pPayloadRecvBuffer->Print (&Serial, true);
  #endif

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_NotFound)
  {
    // Do NOT clear the buffer, because only a part of an incoming message may have been received yet.
    // Clearing would erase this part, and the message would never be completely available.
    m_DataAvailable = false;
    return ::EResult::SUCCESS;
  }

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_ReceiverDeviceIdMismatch)  // Message is not sent to this device. Nothing to do.
    return ::EResult::SUCCESS;

  if (receivedMessageTypeIsReply)
  {
    #ifdef TOBA_DEBUG
    Serial << F("Message is no request, although it should be. Nothing to do.") << endl;
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
    Serial << F("Message is faulty. Creating failure reply.") << endl;
    #endif
    m_ReplyData = UCOPData::CreateReplyData (receivedData, GetTimestamp (), messageResult);
    return result;
  }

  #ifdef TOBA_DEBUG
  Serial << F("Message is a request.") << endl;
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

  result = pWOCO->AnalyzeCommandData (m_RequestData.pPayloadBuffer,
                                      m_RequestData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.AnalyzeCommandData() result=") << WOCO::GetResultText (result) << endl;
  #endif

  m_RequestData.pPayloadBuffer->Clear ();
  #ifdef TOBA_DEBUG
  Serial << F("PayloadRecvBuffer:") << endl;
  m_pPayloadRecvBuffer->Print (&Serial, true);
  #endif

  if (result != ::EResult::SUCCESS)
  {
    m_ReplyData = UCOPData::CreateReplyData (m_RequestData, GetTimestamp (), UCOP::EMessageResult::FAIL_PayloadProcessing);

    DeleteObject (pWOCO);
    m_RequestData.Clear ();
    return result;
  }

  #ifdef TOBA_DEBUG
  Serial << F("Command is valid. Start working...") << endl;
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
                                            replyMessageLength);

  m_pPayloadSendBuffer->Clear ();
  m_ReplyData.Clear ();

  #ifdef TOBA_DEBUG
  Serial << F("UCOP.ComposeReply() result=") << UCOP::GetResultText (result) << endl;
  Serial << F("SendBuffer: bytes used = ") << replyMessageLength << endl;
  m_pSendBuffer->Print (&Serial, true);
  #endif

  if (result == ::EResult::SUCCESS)
  {
    m_pCommStream->write (m_pSendBuffer->get_pData (), replyMessageLength);
    m_pCommStream->flush ();
  }

  m_pSendBuffer->Clear ();

  return result;
}

//--------------------------------------------------------------------
::EResult TOBAWorker::Work ()
{
  if (!m_ReplyData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_pWOCO == nullptr)
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerCommandMissing;

  WOCO* pWORE = nullptr;
  UCOP::EMessageResult messageResult = UCOP::EMessageResult::None;

  ::EResult result = Work_EXEC (pWORE, messageResult);
  if (result != ::EResult::SUCCESS
  ||  messageResult == UCOP::EMessageResult::None)
    messageResult = UCOP::EMessageResult::FAIL_InternalFailure;
  #ifdef TOBA_DEBUG
  Serial << F("Work_EXEC() result=") << TOBA::GetResultText (result) << endl;
  #endif
  DeleteObject (m_pWOCO);

  UCOPData replyData = UCOPData::CreateReplyData (m_RequestData, GetTimestamp (), messageResult);
  replyData.SetPayloadInfo (m_pPayloadSendBuffer);
  m_RequestData.Clear ();

  if (pWORE != nullptr)
  {
    ::EResult result2 = pWORE->ComposeCommandData ( replyData.pPayloadBuffer,
                                                    replyData.PayloadLength);
    #ifdef TOBA_DEBUG
    Serial << F("WOCO.ComposeCommandData() result=") << WOCO::GetResultText (result) << endl;
    Serial << F("PayloadSendBuffer:") << endl;
    m_pPayloadSendBuffer->Print (&Serial, true);
    #endif

    if (result == ::EResult::SUCCESS)
      result = result2;

    DeleteObject (pWORE);
  }

  m_ReplyData = replyData;

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

//--------------------------------------------------------------------
::EResult TOBAWorker::Work_EXEC ( WOCO*&                o_pWORE,
                                  UCOP::EMessageResult& o_MessageResult)
{
  if (m_pWOCO == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;
  if (o_pWORE != nullptr)  // WOrker REply
    return ::EResult::FAIL_Pointer_IsNotZero;

  o_MessageResult = UCOP::EMessageResult::None;
  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::DeviceType:
    o_pWORE = WOCO_DeviceType::CreateReadReply ((uint32_t)get_DeviceType ());
    o_MessageResult = UCOP::EMessageResult::SUCCESS;
    break;

  case WOCO::ECommand::DeviceName:
    o_pWORE = WOCO_DeviceName::CreateReadReply (get_DeviceName (), get_DeviceNameLength ());
    o_MessageResult = UCOP::EMessageResult::SUCCESS;
    break;

  case WOCO::ECommand::AliveCheck:
    o_pWORE = WOCO_AliveCheck::CreateReadReply ();
    o_MessageResult = UCOP::EMessageResult::SUCCESS;
    break;

  default:
    return ::EResult::InProgress;
  }

  return ::EResult::SUCCESS;
}
