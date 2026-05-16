#include <Arduino.h>
#include <Streaming.h>

#include <UOCP.h>
#include <UOCPData.h>
#include <WOCO.h>
#include "WOCO_AliveCheck.h"
#include "WOCO_DigitalPinMode.h"
#include "WOCO_DigitalPinState.h"

const uint8_t c_BufferDefaultValue = 0xDD;

const uint16_t c_EepromOffset = 0;

UOCP* m_pUOCP = nullptr;
WOCO* m_pWOCO = nullptr;

uint8_t m_CommandBuffer[20];
uint8_t m_PayloadSendBuffer[20];
uint8_t m_PayloadRecvBuffer[20];
uint8_t m_SendBuffer[50];
uint8_t m_ReceiveBuffer[80];

UOCP::EChecksumType m_ChecksumType = UOCP::EChecksumType::CRC16;

uint8_t   m_CommandLength            = 0;
uint8_t*  m_pCommandBufferCurrentPos = nullptr;
uint16_t  m_ReceiveBufferWriteIndex  = 0;
uint16_t  m_ReceiveBufferReadIndex   = 0;
bool      m_DataAvailable            = false;
UOCPData  m_RequestData;

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

  m_pUOCP = new UOCP (c_EepromOffset, result);
  Serial << F("UOCP.ctor() result=") << UOCP::GetResultText (result) << endl;
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;

  if (Serial.available ())
  {
    Serial << "Serial available: " << Serial.available () << endl;

    // Receive all available data
    while (Serial.available () > 0
        && m_CommandLength < sizeof (m_CommandBuffer))
    {
      m_CommandBuffer[m_CommandLength++] = Serial.read ();
    }
  }

  if (m_CommandLength > 0)
  {
    Serial << F("CommandBuffer: bytes used = ") << m_CommandLength << endl;
    Memory_PrintLn (m_CommandBuffer, sizeof (m_CommandBuffer));

    uint32_t workerDeviceId     = 0;
    uint16_t wocoCommandNum     = 0;
    bool     wocoActionIsWrite  = false;
    uint8_t  pinNumber          = 0;
    uint8_t  pinMode            = 0;
    bool     pinState           = false;
    RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, workerDeviceId);
    RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, wocoCommandNum);
    RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, wocoActionIsWrite);
    WOCO::ECommand wocoCommand = (WOCO::ECommand)wocoCommandNum;
    switch (wocoCommand)
    {
    case WOCO::ECommand::AliveCheck:
      m_pWOCO = (WOCO*)WOCO_AliveCheck::CreateReadRequest ();
      break;

    case WOCO::ECommand::DigitalPinMode:
      RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, pinNumber);
      if (wocoActionIsWrite)
      {
        RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, pinMode);
        m_pWOCO = WOCO_DigitalPinMode::CreateWriteRequest (pinNumber, pinMode);
      }
      else
        m_pWOCO = WOCO_DigitalPinMode::CreateReadRequest (pinNumber);
      break;

    case WOCO::ECommand::DigitalPinState:
      RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, pinNumber);
      if (wocoActionIsWrite)
      {
        RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), m_pCommandBufferCurrentPos, pinState);
        m_pWOCO = WOCO_DigitalPinState::CreateWriteRequest (pinNumber, pinState);
      }
      else
        m_pWOCO = WOCO_DigitalPinState::CreateReadRequest (pinNumber);
      break;
    }

    uint8_t payloadLength = 0;
    result = m_pWOCO->ComposePayload (m_PayloadSendBuffer,
                                      sizeof (m_PayloadSendBuffer),
                                      payloadLength);
    Serial << F("WOCO->ComposePayload() result=") << WOCO::GetResultText (result) << endl;

    Serial << F("PayloadSendBuffer: bytes used = ") << payloadLength << endl;
    Memory_PrintLn (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer));

    m_RequestData = UOCPData (m_pWOCO->GetActionIsWrite (),
                              workerDeviceId,
                              (uint8_t)m_pWOCO->GetCommand ());
    m_RequestData.SetPayloadInfo (m_PayloadSendBuffer,
                                  sizeof (m_PayloadSendBuffer),
                                  payloadLength);
    uint16_t requestMessageLength = 0;

    result = m_pUOCP->ComposeRequest (m_RequestData,
                                      m_SendBuffer,
                                      sizeof (m_SendBuffer),
                                      requestMessageLength);
    Serial << F("UOCP.ComposeRequest() result=") << UOCP::GetResultText (result) << endl;

    Serial << F("SendBuffer: bytes used = ") << requestMessageLength << endl;
    Memory_PrintLn (m_SendBuffer, sizeof (m_SendBuffer));

    if (result == EResult::SUCCESS)
    {
      Serial << F("Sending data...") << endl;
      Serial1.write (m_SendBuffer, requestMessageLength);
      Serial1.flush ();
    }

    memset (m_CommandBuffer, c_BufferDefaultValue, m_CommandLength);
    Serial << F("CommandBuffer:") << endl;
    Memory_PrintLn (m_CommandBuffer, sizeof (m_CommandBuffer));
    m_CommandLength = 0;

    memset (m_PayloadSendBuffer, c_BufferDefaultValue, payloadLength);
    Serial << F("PayloadSendBuffer:") << endl;
    Memory_PrintLn (m_PayloadSendBuffer, sizeof (m_PayloadSendBuffer));

    memset (m_SendBuffer, c_BufferDefaultValue, requestMessageLength);
    Serial << F("SendBuffer:") << endl;
    Memory_PrintLn (m_SendBuffer, sizeof (m_SendBuffer));
  }

  if (Serial1.available ())
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

  if (m_DataAvailable)
  {
    bool     receivedMessageTypeIsReply = false;
    uint8_t  receivedMessageLength      = 0;
    UOCPData receivedData;
    receivedData.SetPayloadInfo (m_PayloadRecvBuffer,
                                 sizeof (m_PayloadRecvBuffer));

    // Search message in the received data
    result = m_pUCOP->SearchMessage (m_ReceiveBuffer,
                                     sizeof (m_ReceiveBuffer),
                                     m_ReceiveBufferReadIndex,
                                     receivedData,
                                     receivedMessageTypeIsReply,
                                     receivedMessageLength);
    Serial << F("UOCP.SearchMessage() result=") << UOCP::GetResultText (result) << endl;

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
      RingBuffer_SetValueBackward (m_ReceiveBuffer,
                                   sizeof (m_ReceiveBuffer),
                                   m_ReceiveBufferReadIndex,
                                   receivedMessageLength,
                                   c_BufferDefaultValue);
      Serial << F("ReceiveBuffer:") << endl;
      Memory_PrintLn (m_ReceiveBuffer, sizeof (m_ReceiveBuffer));

      // Evaluate reply

      memset (m_PayloadRecvBuffer, c_BufferDefaultValue, receivedData.PayloadLength);
      Serial << F("PayloadRecvBuffer:") << endl;
      Memory_PrintLn (m_PayloadRecvBuffer, sizeof (m_PayloadRecvBuffer));
    }
    else
      m_DataAvailable = false;
  }

  if (!m_DataAvailable)
  {
    Serial.println ("Idle.");
    delay (1000);
  }
}
