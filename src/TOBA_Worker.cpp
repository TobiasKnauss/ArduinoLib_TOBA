#include "TOBA_Worker.h"

//--------------------------------------------------------------------
#define X(name) const char TOBA_Worker::_EResult_##name[] PROGMEM = #name;
#include "TOBA_failures.h"
#undef X

//--------------------------------------------------------------------
#define X(name) _EResult_##name,
const char* const TOBA_Worker::c_EResult_ClassFailures_Names[] PROGMEM =
{
  #include "TOBA_EResult_failures.h"
};
#undef X

//--------------------------------------------------------------------
TOBA_Worker::TOBA_Worker (Stream*    i_pCommStream,
                          uint16_t   i_ReceiveBufferSize,
                          uint16_t   i_SendBufferSize,
                          uint16_t   i_PayloadBuffersSize,
                          ::EResult& o_Result)
{
  if (i_pCommStream == 0)
  {
    o_Result = ::EResult::FAIL_Pointer_IsZero;
    return;
  }
  if (i_ReceiveBufferSize  < c_MinRecvSendBuffersSize
  ||  i_SendBufferSize     < c_MinRecvSendBuffersSize
  ||  i_PayloadBuffersSize < c_MinPayloadRecvSendBuffersSize)
  {
    o_Result = ::EResult::FAIL_Buffer_TooSmall;
    return;
  }

  m_ReceiveBufferSize  = i_ReceiveBufferSize;
  m_SendBufferSize     = i_SendBufferSize;
  m_PayloadBuffersSize = i_PayloadBuffersSize;

  m_pReceiveBuffer     = new uint8_t[m_ReceiveBufferSize];
  m_pSendBuffer        = new uint8_t[m_SendBufferSize];
  m_pPayloadRecvBuffer = new uint8_t[m_PayloadBuffersSize];
  m_pPayloadSendBuffer = new uint8_t[m_PayloadBuffersSize];

  memset (m_pPayloadSendBuffer, c_BufferDefaultValue, m_PayloadBuffersSize);
  memset (m_pPayloadRecvBuffer, c_BufferDefaultValue, m_PayloadBuffersSize);
  memset (m_pSendBuffer       , c_BufferDefaultValue, m_SendBufferSize);
  memset (m_pReceiveBuffer    , c_BufferDefaultValue, m_ReceiveBufferSize);

#ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer     Addr=") << _HEX4((uint16_t)m_ReceiveBuffer)     << " Len=" << m_ReceiveBufferSize << endl;
  Memory_PrintLn (m_ReceiveBuffer, m_ReceiveBufferSize);
  Serial << F("SendBuffer        Addr=") << _HEX4((uint16_t)m_SendBuffer)        << " Len=" << m_SendBufferSize << endl;
  Memory_PrintLn (m_SendBuffer, m_SendBufferSize);
  Serial << F("PayloadRecvBuffer Addr=") << _HEX4((uint16_t)m_PayloadRecvBuffer) << " Len=" << m_PayloadBuffersSize) << endl;
  Memory_PrintLn (m_PayloadRecvBuffer, m_PayloadBuffersSize);
  Serial << F("PayloadSendBuffer Addr=") << _HEX4((uint16_t)m_PayloadSendBuffer) << " Len=" << m_PayloadBuffersSize << endl;
  Memory_PrintLn (m_PayloadSendBuffer, m_PayloadBuffersSize);
#endif

  m_pCommStream = i_pCommStream;

  m_pUCOP = new UCOP (c_EepromAddr_UcopConfig, o_Result);
  if (o_Result != ::EResult::SUCCESS)
  {
    delete (m_pUCOP);
    m_pUCOP = 0;
  }

#ifdef TOBA_DEBUG
  Serial << F("UCOP.ctor() result=") << UCOP::GetResultText (result) << endl;
#endif
}

//--------------------------------------------------------------------
bool TOBA_Worker::get_IsWorking ()
{
  return m_IsWorking;
}

//--------------------------------------------------------------------
const __FlashStringHelper* TOBA_Worker::GetResultText (::EResult i_Result)
{
  if ((uint16_t)i_Result < (uint16_t)EResult::Dummy_FirstClassFailure)
    return Result::GetText (i_Result);
  return (const __FlashStringHelper*)pgm_read_ptr(&c_EResult_ClassFailures_Names[(uint16_t)i_Result - (uint16_t)EResult::Dummy_FirstClassFailure - 1]);
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::AnalyzeData ()
{
  if (!m_DataAvailable)
    return ::EResult::SUCCESS;

  if (m_IsWorking)
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;

  ::EResult result = ::EResult::InProgress;
  bool     receivedMessageTypeIsReply = false;
  uint8_t  receivedMessageLength      = 0;
  UCOPData receivedData;
  receivedData.SetPayloadInfo (m_pPayloadRecvBuffer,
                               m_PayloadBuffersSize);

  // Analyse message in the received data
  uint16_t receiveBufferReadIndexAtAnalyseStart = m_ReceiveBufferReadIndex;
  result = m_pUCOP->SearchMessage (m_pReceiveBuffer,
                                   m_ReceiveBufferSize,
                                   m_ReceiveBufferReadIndex,
                                   receivedData,
                                   receivedMessageTypeIsReply,
                                   receivedMessageLength);

  RingBuffer_SetValue_StartToEnd (m_pReceiveBuffer,
                                  m_ReceiveBufferSize,
                                  receiveBufferReadIndexAtAnalyseStart,
                                  m_ReceiveBufferReadIndex,
                                  c_BufferDefaultValue);

#ifdef UCOP_DEBUG
  Serial << F("UCOP.AnalyseMessage() result=") << UCOP::GetResultText (result) << endl;

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
  Memory_PrintLn (m_ReceiveBuffer, m_ReceiveBufferSize);
  Serial << F("PayloadRecvBuffer: bytes used = ") << receivedData.PayloadLength << endl;
  Memory_PrintLn (m_PayloadRecvBuffer, m_PayloadBuffersSize);
#endif

  if (receivedMessageTypeIsReply)  // Flag is only set if a message was found.
  {
#ifdef UCOP_DEBUG
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
    m_ReplyData = UCOPData (m_RequestData.ActionIsWrite,
                            m_RequestData.RemoteDeviceId,
                            m_RequestData.MessageId,
                            0,
                            m_RequestData.CommandId,
                            messageResult);
  }

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_NotFound)
  {
    ClearReceiveBuffer ();
    m_DataAvailable = false;
    return ::EResult::SUCCESS;
  }

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_ReceiverDeviceIdMismatch)  // Message is not sent to this device. Nothing to do.
    return ::EResult::SUCCESS;

  Serial << F("Message is a request.");
  m_RequestData = receivedData;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::AnalyzeRequest ()
{
  EResult result;
  UCOP::EMessageResult messageResult = UCOP::EMessageResult::None;

  result = WOCO::Create ((WOCO::ECommand)m_RequestData.CommandId,
                         false,
                         m_RequestData.ActionIsWrite,
                         &m_WOCO);
  Serial << F("WOCO.Create() result=") << WOCO::GetResultText (result) << endl;

  if (result != EResult::SUCCESS)
  {
    messageResult = UCOP::EMessageResult::FAIL_CommandNotSupported;
  }
  
  result = pWOCO->AnalyzePayload (m_RequestData.pPayloadBuffer,
                                  m_RequestData.PayloadBufferLength,
                                  m_RequestData.PayloadLength);
  Serial << F("WOCO.AnalyzePayload() result=") << WOCO::GetResultText (result) << endl;

  memset (m_RequestData.pPayloadBuffer, c_BufferDefaultValue, m_RequestData.PayloadLength);
  Serial << F("PayloadRecvBuffer:") << endl;
  Memory_PrintLn (m_PayloadRecvBuffer, m_PayloadBuffersSize);

  if (result != EResult::SUCCESS)
    messageResult = UCOP::EMessageResult::FAIL_PayloadProcessing;
  

  if (result == EResult::SUCCESS)
  {
    Serial << F("Command is valid. Start working...");
    m_pWOCO = pWOCO;
    m_IsWorking = true;
  }

  if (result != EResult::SUCCESS)
  {
    m_ReplyData = new UCOPData (m_RequestData.ActionIsWrite,
                                m_RequestData.RemoteDeviceId,
                                m_RequestData.MessageId,
                                0,
                                m_RequestData.CommandId,
                                messageResult);
  }
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::ReadData ()
{
  if (m_pCommStream == 0)
    return (::EResult)EResult::FAIL_TOBA_CommStream_NotReady;
  if (!m_pCommStream->available ())
    return ::EResult::SUCCESS;

  Serial << "CommStream available: " << m_pCommStream->available () << endl;

  // Receive all available data
  while (m_pCommStream->available ())
  {
    m_pReceiveBuffer[m_ReceiveBufferWriteIndex++] = m_pCommStream->read ();
    if (m_ReceiveBufferWriteIndex >= m_ReceiveBufferSize)
      m_ReceiveBufferWriteIndex = 0;
  }

  Serial << F("ReceiveBuffer: position = ") << m_ReceiveBufferWriteIndex << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_ReceiveBufferSize);

  m_DataAvailable = true;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::SendReply ()
{
  if (m_ReplyData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_NoReplyAvailable;
  
  ::EResult result;
  uint16_t replyMessageLength = 0;

  result = m_pUCOP->ComposeReply (m_ReplyData,
                                  m_SendBuffer,
                                  sizeof (m_SendBuffer),
                                  replyMessageLength);
  Serial << F("UCOP.ComposeReply() result=") << UCOP::GetResultText (result) << endl;

  Serial << F("SendBuffer: bytes used = ") << replyMessageLength << endl;
  Memory_PrintLn (m_SendBuffer, sizeof (m_SendBuffer));

  if (result == EResult::SUCCESS)
  {
    Serial1.write (m_SendBuffer, replyMessageLength);
    Serial1.flush ();
  }

  memset (m_PayloadSendBuffer, c_BufferDefaultValue, replyData.PayloadLength);
  memset (m_SendBuffer,        c_BufferDefaultValue, replyMessageLength);

  m_IsWorking = false;
  m_pWOCO = 0;

}

//--------------------------------------------------------------------
::EResult TOBA_Worker::Work ()
{
  WOCO* pWORE = 0;  // Reply
  switch (m_pWOCO->GetCommand ())
  {
  case WOCO::ECommand::AliveCheck:
    pWORE = WOCO_AliveCheck::CreateReadReply ();
    break;
  }

    Serial << F("PayloadSendBuffer:") << endl;
    Memory_PrintLn (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer));

    UCOPData replyData = UCOPData (m_RequestData.ActionIsWrite,
                                   m_RequestData.RemoteDeviceId,
                                   m_RequestData.MessageId,
                                   m_RequestData.Timestamp,
                                   m_RequestData.CommandId,
                                   UCOP::EMessageResult::Success);
    replyData.SetPayloadInfo (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer), m_RequestData.PayloadLength);

}
