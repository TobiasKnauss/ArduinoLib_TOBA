#include <EEPROM.h>
#include "TOBA_Worker.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
#define X(name) const char TOBA_Worker::_EResult_##name[] PROGMEM = #name;
#include "TOBA_EResult_failures.h"
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
                          char*      i_pWorkerName,
                          uint8_t    i_WorkerNameLength,
                          UCOP*      i_pUCOP,
                          ::EResult& o_Result)
{
  o_Result = CommonConstructor_Ext (i_pCommStream);
  if (o_Result != ::EResult::SUCCESS)
    return;

  o_Result = CommonConstructor_Cfg (i_ReceiveBufferSize,
                                    i_SendBufferSize,
                                    i_PayloadBuffersSize,
                                    i_pWorkerName,
                                    i_WorkerNameLength,
                                    i_pUCOP);
  if (o_Result != ::EResult::SUCCESS)
    return;
}

//--------------------------------------------------------------------
TOBA_Worker::TOBA_Worker (Stream*    i_pCommStream,
                          uint16_t   i_EepromAddress,
                          ::EResult& o_Result)
{
  o_Result = CommonConstructor_Ext (i_pCommStream);
  if (o_Result != ::EResult::SUCCESS)
    return;

  o_Result = ReadConfigFromEEPROM (i_EepromAddress);
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::CommonConstructor_Ext (Stream* i_pCommStream)
{
  if (i_pCommStream == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  m_pCommStream = i_pCommStream;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::CommonConstructor_Cfg (uint16_t i_ReceiveBufferSize,
                                              uint16_t i_SendBufferSize,
                                              uint16_t i_PayloadBuffersSize,
                                              char*    i_pWorkerName,
                                              uint8_t  i_WorkerNameLength,
                                              UCOP*    i_pUCOP)
{
  if (i_pWorkerName == nullptr
  ||  i_pUCOP       == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  if (i_ReceiveBufferSize  < c_MinRecvSendBuffersSize
  ||  i_SendBufferSize     < c_MinRecvSendBuffersSize
  ||  i_PayloadBuffersSize < c_MinPayloadRecvSendBuffersSize)
    return ::EResult::FAIL_Buffer_TooSmall;

  m_ReceiveBufferSize  = i_ReceiveBufferSize;
  m_SendBufferSize     = i_SendBufferSize;
  m_PayloadBuffersSize = i_PayloadBuffersSize;

  if (!Memory_Allocate (m_pReceiveBuffer    , m_ReceiveBufferSize , c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pSendBuffer       , m_SendBufferSize    , c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pPayloadRecvBuffer, m_PayloadBuffersSize, c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pPayloadSendBuffer, m_PayloadBuffersSize, c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;

  #ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer     Addr=") << _HEX4((uint16_t)m_pReceiveBuffer)     << " Len=" << m_ReceiveBufferSize << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_ReceiveBufferSize);
  Serial << F("SendBuffer        Addr=") << _HEX4((uint16_t)m_pSendBuffer)        << " Len=" << m_SendBufferSize << endl;
  Memory_PrintLn (m_pSendBuffer, m_SendBufferSize);
  Serial << F("PayloadRecvBuffer Addr=") << _HEX4((uint16_t)m_pPayloadRecvBuffer) << " Len=" << m_PayloadBuffersSize << endl;
  Memory_PrintLn (m_pPayloadRecvBuffer, m_PayloadBuffersSize);
  Serial << F("PayloadSendBuffer Addr=") << _HEX4((uint16_t)m_pPayloadSendBuffer) << " Len=" << m_PayloadBuffersSize << endl;
  Memory_PrintLn (m_pPayloadSendBuffer, m_PayloadBuffersSize);
  #endif

  memset (m_WorkerName, 0x00, sizeof (m_WorkerName));
  if (i_pWorkerName != nullptr)
    memcpy (m_WorkerName, i_pWorkerName, min (sizeof (m_WorkerName) - 1, i_WorkerNameLength));

  m_pUCOP = i_pUCOP;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBA_Worker::~TOBA_Worker ()
{
  if (m_NeedToDeleteUCOP)
    DeleteObject (m_pUCOP);

  DeleteObject (m_pReceiveBuffer);
  DeleteObject (m_pSendBuffer);
  DeleteObject (m_pPayloadRecvBuffer);
  DeleteObject (m_pPayloadSendBuffer);
}

//--------------------------------------------------------------------
uint16_t TOBA_Worker::get_EepromAddress ()
{
  return m_EepromAddress;
}

//--------------------------------------------------------------------
bool TOBA_Worker::get_ExistsReply ()
{
  return !m_ReplyData.IsEmpty ();
}

//--------------------------------------------------------------------
bool TOBA_Worker::get_IsBusy ()
{
  return !m_RequestData.IsEmpty ()
      || !m_ReplyData.IsEmpty ()
      || m_pWOCO != nullptr;
}

//--------------------------------------------------------------------
bool TOBA_Worker::get_IsWorking ()
{
  return m_pWOCO != nullptr;
}

//--------------------------------------------------------------------
char* TOBA_Worker::get_WorkerName ()
{
  return m_WorkerName;
}

//--------------------------------------------------------------------
uint8_t TOBA_Worker::get_WorkerNameLength ()
{
  return strlen (m_WorkerName);
}

//--------------------------------------------------------------------
TOBA_Worker::EWorkerType TOBA_Worker::get_WorkerType ()
{
  return EWorkerType::BuiltIn_Basic;
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
  if (get_IsBusy ())
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;

  if (!m_DataAvailable)
    return ::EResult::SUCCESS;

  ::EResult result = ::EResult::InProgress;
  bool     receivedMessageTypeIsReply = false;
  uint8_t  receivedMessageLength      = 0;
  UCOPData receivedData;
  receivedData.SetPayloadInfo (m_pPayloadRecvBuffer,
                               m_PayloadBuffersSize);

  // Analyze message in the received data
  uint16_t receiveBufferReadIndexAtAnalyzeStart = m_ReceiveBufferReadIndex;
  result = m_pUCOP->SearchMessage (m_pReceiveBuffer,
                                   m_ReceiveBufferSize,
                                   m_ReceiveBufferReadIndex,
                                   receivedData,
                                   receivedMessageTypeIsReply,
                                   receivedMessageLength);

  // In the ring buffer, clear the part that contains the found message.
  RingBuffer_SetValue_StartToEnd (m_pReceiveBuffer,
                                  m_ReceiveBufferSize,
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
  Memory_PrintLn (m_pReceiveBuffer, m_ReceiveBufferSize);
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
    m_ReplyData = UCOPData (receivedData.ActionIsWrite,
                            receivedData.RemoteDeviceId,
                            receivedData.MessageId,
                            GetTimestamp (),
                            receivedData.CommandId,
                            messageResult);
  }

  if ((UCOP::EResult)result == UCOP::EResult::FAIL_UCOP_Message_NotFound)
  {
    memset (m_pReceiveBuffer, c_BufferDefaultValue, m_ReceiveBufferSize);
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
::EResult TOBA_Worker::AnalyzeRequest ()
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
    m_ReplyData = UCOPData (m_RequestData.ActionIsWrite,
                            m_RequestData.RemoteDeviceId,
                            m_RequestData.MessageId,
                            GetTimestamp (),
                            m_RequestData.CommandId,
                            UCOP::EMessageResult::FAIL_CommandNotSupported);

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
    m_ReplyData = UCOPData (m_RequestData.ActionIsWrite,
                            m_RequestData.RemoteDeviceId,
                            m_RequestData.MessageId,
                            GetTimestamp (),
                            m_RequestData.CommandId,
                            UCOP::EMessageResult::FAIL_PayloadProcessing);

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
uint32_t TOBA_Worker::GetTimestamp ()
{
  return 0;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::ReadData ()
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
    if (m_ReceiveBufferWriteIndex >= m_ReceiveBufferSize)
      m_ReceiveBufferWriteIndex = 0;
  }

  #ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer: position = ") << m_ReceiveBufferWriteIndex << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_ReceiveBufferSize);
  #endif

  m_DataAvailable = true;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::SendReply ()
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
                                            m_SendBufferSize,
                                            replyMessageLength);

  memset (m_pPayloadSendBuffer, c_BufferDefaultValue, m_ReplyData.PayloadLength);

  #ifdef TOBA_DEBUG
  Serial << F("UCOP.ComposeReply() result=") << UCOP::GetResultText (result) << endl;
  Serial << F("SendBuffer: bytes used = ") << replyMessageLength << endl;
  Memory_PrintLn (m_pSendBuffer, m_SendBufferSize);
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
::EResult TOBA_Worker::Work ()
{
  if (!m_ReplyData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_WorkerIsBusy;
  if (m_pWOCO == nullptr)
    return (::EResult)EResult::FAIL_TOBA_WorkerCommandMissing;

  WOCO* pWORE = nullptr;  // WOrker REply
  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::WorkerType:
    pWORE = WOCO_WorkerType::CreateReadReply ((uint32_t)get_WorkerType ());
    break;

  case WOCO::ECommand::WorkerName:
    pWORE = WOCO_WorkerName::CreateReadReply (get_WorkerName (), get_WorkerNameLength ());
    break;

  case WOCO::ECommand::AliveCheck:
    pWORE = WOCO_AliveCheck::CreateReadReply ();
    break;

  default:
    return ::EResult::InProgress;
  }

  m_ReplyData = UCOPData (m_RequestData.ActionIsWrite,
                          m_RequestData.RemoteDeviceId,
                          m_RequestData.MessageId,
                          GetTimestamp (),
                          m_RequestData.CommandId,
                          UCOP::EMessageResult::SUCCESS);
  m_ReplyData.SetPayloadInfo (m_pPayloadSendBuffer, m_PayloadBuffersSize);

  ::EResult result = pWORE->ComposeCommandData (m_ReplyData.pPayloadBuffer,
                                                m_ReplyData.PayloadBufferLength,
                                                m_ReplyData.PayloadLength);
  #ifdef TOBA_DEBUG
  Serial << F("WOCO.ComposeCommandData() result=") << UCOP::GetResultText (result) << endl;
  #endif

  DeleteObject (m_pWOCO);
  DeleteObject (pWORE);

  return result;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::WriteConfigToEEPROM (uint16_t i_Address)
{
  if (c_EepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  EEPROM.put (i_Address +  0, m_ReceiveBufferSize);
  EEPROM.put (i_Address +  2, m_SendBufferSize);
  EEPROM.put (i_Address +  4, m_PayloadBuffersSize);
  for (uint8_t index = 0; index < sizeof (m_WorkerName) - 1; index++)
    EEPROM.write (i_Address + 6 + index, m_WorkerName[index]);
  EEPROM.put (i_Address + 38, m_pUCOP == nullptr ? 0 : m_pUCOP->get_EepromAddress ());

  uint8_t  byteValue = 0;
  uint16_t checksum = m_Crc16.modbus (&byteValue, 1);
  for (int index = 0; index < c_EepromConfigDataSize; index++)
  {
    byteValue = EEPROM.read (i_Address + index);
    checksum = m_Crc16.modbus_upd (&byteValue, 1);
  }
  EEPROM.put (i_Address + 40, checksum);

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBA_Worker::ReadConfigFromEEPROM (uint16_t i_Address)
{
  if (c_EepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t receiveBufferSize;
  uint16_t sendBufferSize;
  uint16_t payloadBuffersSize;
  char     workerName[sizeof (m_WorkerName)];
  uint8_t  workerNameLength;
  uint16_t eepromAddr_UcopConfig;
  uint16_t configChecksumCRC16;

  EEPROM.get (i_Address +  0, receiveBufferSize);
  EEPROM.get (i_Address +  2, sendBufferSize);
  EEPROM.get (i_Address +  4, payloadBuffersSize);
  for (uint8_t index = 0; index < sizeof (workerName) - 1; index++)
    workerName[index] = EEPROM.read (i_Address + 6 + index);
  EEPROM.get (i_Address + 38, eepromAddr_UcopConfig);
  EEPROM.get (i_Address + 40, configChecksumCRC16);
  workerNameLength = (uint8_t)strlen (workerName);

  uint8_t  byteValue = 0;
  uint16_t checksum = m_Crc16.modbus (&byteValue, 1);
  for (int index = 0; index < c_EepromConfigDataSize; index++)
  {
    byteValue = EEPROM.read (i_Address + index);
    checksum = m_Crc16.modbus_upd (&byteValue, 1);
  }
  if (checksum != configChecksumCRC16)
    return ::EResult::FAIL_Device_ConfigChecksumWrong;

  ::EResult result;
  UCOP* pUCOP = new UCOP (eepromAddr_UcopConfig, result);
  m_NeedToDeleteUCOP = true;
  #ifdef TOBA_DEBUG
  Serial << F("UCOP.ctor() result=") << UCOP::GetResultText (result) << endl;
  #endif
  if (result != ::EResult::SUCCESS)
  {
    DeleteObject (pUCOP);
    return result;
  }

  result = CommonConstructor_Cfg (receiveBufferSize,
                                  sendBufferSize,
                                  payloadBuffersSize,
                                  workerName,
                                  workerNameLength,
                                  pUCOP);

  return result;
}
