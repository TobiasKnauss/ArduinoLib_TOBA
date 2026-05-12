#include <Arduino.h>
#include <Streaming.h>

#include <STUC.h>
#include <StucData.h>
#include <WOCO.h>
#include <WOCO_AliveCheck.h>
#include <WOCO_DigitalPinState.h>
#include <WOCO_DigitalPinMode.h>

const uint8_t c_BufferDefaultValue = 0xDD;
const uint16_t c_EepromAddr_StucConfig = 0;

STUC* m_pSTUC = 0;
WOCO* m_pWOCO = 0;  // Command

uint8_t m_PayloadSendBuffer[20];
uint8_t m_PayloadRecvBuffer[20];
uint8_t m_SendBuffer[50];
uint8_t m_ReceiveBuffer[80];

uint16_t  m_ReceiveBufferWriteIndex = 0;
uint16_t  m_ReceiveBufferReadIndex  = 0;
bool      m_DataAvailable           = false;
bool      m_IsWorking               = false;
StucData  m_RequestData;
StucData* m_ReplyData   = 0;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  memset (m_PayloadSendBuffer, 0xFF, sizeof (m_PayloadSendBuffer));
  memset (m_PayloadRecvBuffer, 0xFF, sizeof (m_PayloadRecvBuffer));
  memset (m_SendBuffer       , 0xFF, sizeof (m_SendBuffer));
  memset (m_ReceiveBuffer    , 0xFF, sizeof (m_ReceiveBuffer));

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  Serial << F("PayloadSendBuffer Addr=") << _HEX4((uint16_t)m_PayloadSendBuffer) << " Len=" << sizeof (m_PayloadSendBuffer) << endl;
  Memory_PrintLn (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer));
  Serial << F("PayloadRecvBuffer Addr=") << _HEX4((uint16_t)m_PayloadRecvBuffer) << " Len=" << sizeof (m_PayloadRecvBuffer) << endl;
  Memory_PrintLn (m_PayloadRecvBuffer, sizeof (m_PayloadRecvBuffer));
  Serial << F("SendBuffer        Addr=") << _HEX4((uint16_t)m_SendBuffer)        << " Len=" << sizeof (m_SendBuffer) << endl;
  Memory_PrintLn (m_SendBuffer, sizeof (m_SendBuffer));
  Serial << F("ReceiveBuffer     Addr=") << _HEX4((uint16_t)m_ReceiveBuffer)     << " Len=" << sizeof (m_ReceiveBuffer) << endl;
  Memory_PrintLn (m_ReceiveBuffer, sizeof (m_ReceiveBuffer));

  m_pSTUC = new STUC (c_EepromAddr_StucConfig, result);
  Serial << F("STUC.ctor() result=") << STUC::GetResultText (result) << endl;
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;

  if (Serial1.available ()
  &&  !m_IsWorking)
  {
    Serial << "Serial1 available: " << Serial1.available () << endl;

    // Receive all available data
    while (Serial1.available ())
    {
      m_ReceiveBuffer[m_ReceiveBufferWriteIndex++] = Serial1.read ();
      if (m_ReceiveBufferWriteIndex >= sizeof (m_ReceiveBuffer))
        m_ReceiveBufferWriteIndex = 0;
    }

    Serial << F("ReceiveBuffer: position = ") << m_ReceiveBufferWriteIndex << endl;
    Memory_PrintLn (m_ReceiveBuffer, sizeof (m_ReceiveBuffer));

    m_DataAvailable = true;
  }

  if (m_DataAvailable
  &&  !m_IsWorking)
  {
    bool     receivedMessageTypeIsReply = false;
    uint8_t  receivedMessageLength      = 0;
    StucData receivedData;
    receivedData.SetPayloadInfo (m_PayloadRecvBuffer,
                                 sizeof (m_PayloadRecvBuffer));

    // Analyse message in the received data
    result = m_pSTUC->AnalyseMessage (m_ReceiveBuffer,
                                      sizeof (m_ReceiveBuffer),
                                      m_ReceiveBufferReadIndex,
                                      receivedData,
                                      receivedMessageTypeIsReply,
                                      receivedMessageLength);
    Serial << F("STUC.AnalyseMessage() result=") << STUC::GetResultText (result) << endl;

    Serial << F("Message Type is Reply: ") << receivedMessageTypeIsReply          << endl;
    Serial << F("Action is Write:       ") << receivedData.ActionIsWrite          << endl;
    Serial << F("Remote Device Id:      ") << _HEX8 (receivedData.RemoteDeviceId) << endl;
    Serial << F("Message Id:            ") << receivedData.MessageId              << endl;
    Serial << F("Timestamp:             ") << receivedData.Timestamp              << endl;
    Serial << F("CommandId:             ") << receivedData.CommandId              << endl;
    Serial << F("Result:                ") << (uint8_t)receivedData.MessageResult << endl;
    Serial << F("Payload Data Length:   ") << receivedData.PayloadLength          << endl;
    Serial << F("Message Length:        ") << receivedMessageLength               << endl;

    Serial << F("PayloadRecvBuffer: bytes used = ") << receivedData.PayloadLength << endl;
    Memory_PrintLn (m_PayloadRecvBuffer, sizeof (m_PayloadRecvBuffer));

    if (result == EResult::SUCCESS)
    {
      m_RequestData = receivedData;

      RingBuffer_SetValueBackward (m_ReceiveBuffer,
                                   sizeof (m_ReceiveBuffer),
                                   m_ReceiveBufferReadIndex,
                                   receivedMessageLength,
                                   c_BufferDefaultValue);
      Serial << F("ReceiveBuffer:") << endl;
      Memory_PrintLn (m_ReceiveBuffer, sizeof (m_ReceiveBuffer));

      if (!receivedMessageTypeIsReply)
      {
        Serial << F("Message is a request. Analyze Command...");
        m_IsWorking = true;

        WOCO* pWOCO = 0;
        result = WOCO::Create ((WOCO::ECommand)m_RequestData.CommandId,
                               receivedMessageTypeIsReply,
                               m_RequestData.ActionIsWrite,
                               pWOCO);
        Serial << F("WOCO.Create() result=") << WOCO::GetResultText (result) << endl;

        STUC::EMessageResult messageResult = STUC::EMessageResult::None;
        if (result == EResult::SUCCESS)
        {
          result = pWOCO->AnalyzePayload (m_RequestData.pPayloadBuffer,
                                          m_RequestData.PayloadBufferLength,
                                          m_RequestData.PayloadLength);
          Serial << F("WOCO.AnalyzePayload() result=") << WOCO::GetResultText (result) << endl;
          if (result != EResult::SUCCESS)
            messageResult = STUC::EMessageResult::FAIL_PayloadProcessing;
        }
        else
          messageResult = STUC::EMessageResult::FAIL_CommandNotSupported;

        if (result == EResult::SUCCESS)
        {
          Serial << F("Command is valid. Start working...");
          m_pWOCO = pWOCO;
          m_IsWorking = true;
        }

        if (result != EResult::SUCCESS)
        {
          m_ReplyData = new StucData (m_RequestData.ActionIsWrite,
                                      m_RequestData.RemoteDeviceId,
                                      m_RequestData.MessageId,
                                      0,
                                      m_RequestData.CommandId,
                                      messageResult);
        }
      }
      else
      {
        Serial << F("Message is no request. Nothing to do.");
        // Sending a reply is useless since no reply is expected.
      }
    }
    else
    {
      m_DataAvailable = false;

      STUC::EMessageResult messageResult = STUC::GetMessageResultForAnalysisResult (result);
      if (messageResult != STUC::EMessageResult::None)
      {
        m_ReplyData = new StucData (m_RequestData.ActionIsWrite,
                                    m_RequestData.RemoteDeviceId,
                                    m_RequestData.MessageId,
                                    0,
                                    m_RequestData.CommandId,
                                    messageResult);
      }
    }

    memset (m_PayloadRecvBuffer, c_BufferDefaultValue, m_RequestData.PayloadLength);
    Serial << F("PayloadRecvBuffer:") << endl;
    Memory_PrintLn (m_PayloadRecvBuffer, sizeof (m_PayloadRecvBuffer));
  }

  if (m_IsWorking)
  {
    WOCO* pWORE = 0;  // Reply
    switch (m_pWOCO->GetCommand ())
    {
    case WOCO::ECommand::AliveCheck:
      pWORE = WOCO_AliveCheck::CreateReadReply ();
      break;

    case WOCO::ECommand::DigitalPinMode:
      {
        WOCO_DigitalPinMode* pWOCO_DigitalPinMode = (WOCO_DigitalPinMode*)m_pWOCO;
        if (pWOCO_DigitalPinMode->GetActionIsWrite ())
        {
          pinMode (pWOCO_DigitalPinMode->GetPinNumber (),
                   pWOCO_DigitalPinMode->GetPinMode ());
          pWORE = WOCO_DigitalPinMode::CreateWriteReply ();
        }
        else
        {
          uint8_t currentPinMode = queryPinMode (pWOCO_DigitalPinMode->GetPinNumber ());
          pWORE = WOCO_DigitalPinMode::CreateReadReply(pWOCO_DigitalPinMode->GetPinNumber (), currentPinMode);
        }
      }
      break;

    case WOCO::ECommand::DigitalPinState:
      {
        WOCO_DigitalPinState* pWOCO_DigitalPinState = (WOCO_DigitalPinState*)m_pWOCO;
        if (pWOCO_DigitalPinState->GetActionIsWrite ())
        {
          digitalWrite (pWOCO_DigitalPinState->GetPinNumber (),
                        pWOCO_DigitalPinState->GetPinState ());
        }
        else
        {
          bool ioState = digitalRead (pWOCO_DigitalPinState->GetPinNumber ());
          pWORE = WOCO_DigitalPinState::CreateReadReply (pWOCO_DigitalPinState->GetPinNumber (), ioState);
        }
      }
      break;

    default:
      
      break;
    }

    Serial << F("PayloadSendBuffer:") << endl;
    Memory_PrintLn (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer));

    StucData replyData = StucData (m_RequestData.ActionIsWrite,
                                   m_RequestData.RemoteDeviceId,
                                   m_RequestData.MessageId,
                                   m_RequestData.Timestamp,
                                   m_RequestData.CommandId,
                                   STUC::EMessageResult::Success);
    replyData.SetPayloadInfo (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer), m_RequestData.PayloadLength);

    uint16_t replyMessageLength = 0;
    result = m_pSTUC->ComposeReply (replyData,
                                    m_SendBuffer,
                                    sizeof (m_SendBuffer),
                                    replyMessageLength);
    Serial << F("STUC.ComposeReply() result=") << STUC::GetResultText (result) << endl;

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

  if (m_ReplyData != 0)
  {

  }
  
  if (!m_DataAvailable
  &&  !m_IsWorking)
  {
    Serial.println ("Idle.");
    delay (1000);
  }
}
