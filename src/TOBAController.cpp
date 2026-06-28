#include "TOBAController.h"
#include "TOBAControllerConfig.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
::EResult TOBAController::Create (Stream*               i_pCommStream,
                                  UCOP*                 i_pUCOP,
                                  TOBAControllerConfig* i_pConfig,
                                  TOBAController*&      o_pController)
{
  o_pController = nullptr;

  TOBADevice* pDevice = nullptr;
  ::EResult result = TOBADevice::Create (i_pCommStream, i_pUCOP, i_pConfig, pDevice);
  if (result != ::EResult::SUCCESS)
    return result;

  o_pController = (TOBAController*)pDevice;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBAController::TOBAController ()
{
}

//--------------------------------------------------------------------
TOBAController::~TOBAController ()
{
}

//--------------------------------------------------------------------
TOBAControllerConfig* TOBAController::get_Config ()
{
  return m_pConfig;
}

//--------------------------------------------------------------------
::EResult TOBAController::AnalyzeData ()
{
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

  if (!receivedMessageTypeIsReply)
  {
    #ifdef TOBA_DEBUG
    Serial << F("Message is no reply, although it should be. Nothing to do.") << endl;
    #endif
    return ::EResult::SUCCESS;
  }

  #ifdef TOBA_DEBUG
  Serial << F("Message is a reply.") << endl;
  #endif
  m_ReplyData = receivedData;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAController::AnalyzeReply (WOCO*& o_pWORE)
{
  if (m_RequestData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_RequestMissing;
  if (m_ReplyData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_ReplyMissing;
  if (m_pWOCO == nullptr)
    return (::EResult)TOBA::EResult::FAIL_TOBA_WorkerCommandMissing;

  ::EResult result;
  WOCO* pWORE = nullptr;

  result = WOCO::Create ((WOCO::ECommand)m_ReplyData.CommandId,
                         WOCO::TYPE_Reply,
                         m_ReplyData.ActionIsWrite,
                         pWORE);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.Create() result=") << WOCO::GetResultText (result) << endl;
  #endif

  if (result != ::EResult::SUCCESS)
  {
    DeleteObject (pWORE);
    m_RequestData.Clear ();
    m_ReplyData  .Clear ();
    return result;
  }

  result = m_pWOCO->AnalyzeCommandData (m_RequestData.pPayloadBuffer,
                                        m_RequestData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.AnalyzeCommandData() result=") << WOCO::GetResultText (result) << endl;
  #endif

  m_ReplyData.pPayloadBuffer->Clear ();
  #ifdef TOBA_DEBUG
  Serial << F("PayloadRecvBuffer:") << endl;
  m_pPayloadRecvBuffer->Print (&Serial, true);
  #endif

  if (result != ::EResult::SUCCESS)
  {
    DeleteObject (pWORE);
    m_RequestData.Clear ();
    m_ReplyData  .Clear ();
    return result;
  }

  #ifdef TOBA_DEBUG
  Serial << F("Command Reply is valid.");
  #endif
  o_pWORE = pWORE;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
void TOBAController::Clear ()
{
  TOBAController::Clear ();
  ClearReqRep ();
}

//--------------------------------------------------------------------
void TOBAController::ClearReqRep ()
{
  m_ReplyData  .Clear ();
  m_RequestData.Clear ();
}

//--------------------------------------------------------------------
::EResult TOBAController::CreateRequest ( uint32_t  i_WorkerDeviceId,
                                          WOCO*     i_pWOCO)
{
  if (i_pWOCO == nullptr)
    return (::EResult)::EResult::FAIL_Pointer_IsZero;
  if (!m_RequestData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_ControllerIsBusy;

  UCOPData requestData = UCOPData ( i_pWOCO->get_ActionIsWrite (),
                                    i_WorkerDeviceId,
                                    (uint16_t)i_pWOCO->get_Command ());
  requestData.SetPayloadInfo (m_pPayloadSendBuffer);

  EResult result = i_pWOCO->ComposeCommandData (requestData.pPayloadBuffer,
                                                requestData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.ComposeCommandData() result=") << WOCO::GetResultText (result) << endl;
  Serial << F("PayloadSendBuffer:") << endl;
  m_pPayloadSendBuffer->Print (&Serial, true);
  #endif
  if (result != ::EResult::SUCCESS)
    return result;

  m_pWOCO       = i_pWOCO;
  m_RequestData = requestData;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAController::SendRequest ()
{
  if (m_RequestData.IsEmpty ())
    return (::EResult)TOBA::EResult::FAIL_TOBA_RequestMissing;

  uint16_t requestMessageLength = 0;
  ::EResult result = m_pUCOP->ComposeRequest (m_RequestData,
                                              m_pSendBuffer,
                                              requestMessageLength);

  #ifdef TOBA_DEBUG
  Serial << F("UCOP.ComposeRequest() result=") << UCOP::GetResultText (result) << endl;
  Serial << F("SendBuffer: bytes used = ") << requestMessageLength << endl;
  m_pSendBuffer->Print (&Serial, true);
  #endif

  if (result == ::EResult::SUCCESS)
  {
    m_pCommStream->write (m_pSendBuffer->get_pData (), requestMessageLength);
    m_pCommStream->flush ();
  }
  else
  {
    m_RequestData.Clear ();
    m_pWOCO = nullptr;
  }

  m_pSendBuffer->Clear ();

  return result;
}

//--------------------------------------------------------------------
::EResult TOBAController::Init (Stream*           i_pCommStream,
                                UCOP*             i_pUCOP,
                                TOBADeviceConfig* i_pConfig)
{
  ::EResult result = TOBADevice::Init (i_pCommStream, i_pUCOP, i_pConfig);
  if (result != ::EResult::SUCCESS)
    return result;

  m_pConfig = (TOBAControllerConfig*)i_pConfig;

  return ::EResult::SUCCESS;
}
