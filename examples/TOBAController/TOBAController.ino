#include <Arduino.h>
#include <Streaming.h>

#include <WOCO_AliveCheck.h>
#include <WOCO_DeviceName.h>
#include <WOCO_DeviceType.h>
#include <WOCO_DigitalIOMode.h>
#include <WOCO_DigitalIOState.h>
#include <TOBAController.h>
#include <TOBAControllerConfig.h>

const uint16_t c_Sleep_ms = 1000;

const uint16_t c_EepromAddr_ControllerConfig = 16;

WOCO* m_pWOCO = nullptr;
WOCO* m_pWORE = nullptr;
TOBADeviceConfig* m_pTOBADeviceConfig = nullptr;
TOBAController*   m_pTOBAController   = nullptr;

uint8_t m_CommandLength = 0;
uint8_t m_CommandBuffer[20];

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (115200);
  Serial1.begin (9600);
  delay (2000);

  result = TOBADeviceConfig::Create (0, m_pTOBADeviceConfig);
  Serial << F("TOBADeviceConfig.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  result = TOBAController::Create (&Serial1, nullptr, (TOBAControllerConfig*)m_pTOBADeviceConfig, m_pTOBAController);
  Serial << F("TOBAController.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  Serial << endl << F("TOBA Config:") << endl;
  m_pTOBAController->get_Config ()->Print ();
  Serial << endl << F("UCOP Config:") << endl;
  m_pTOBAController->get_UCOPConfig ()->Print ();
}

//--------------------------------------------------------------------
void loop ()
{
  if (m_pTOBAController == nullptr)
  {
    Serial << "Setup failed!" << endl;
    return;
  }

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

  if (m_pWOCO == nullptr
  &&  m_CommandLength > 0)
  {
    Serial << F("CommandBuffer: bytes used = ") << m_CommandLength << endl;
    Memory_PrintLn (m_CommandBuffer, sizeof (m_CommandBuffer));

    uint8_t* pCommandBufferCurrentPos = m_CommandBuffer;
    uint32_t workerDeviceId     = 0;
    uint16_t wocoCommandNum     = 0;
    bool     wocoActionIsWrite  = false;
    uint8_t  pinNumber          = 0;
    uint8_t  pinMode            = 0;
    bool     pinState           = false;
    RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, workerDeviceId);
    RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, wocoCommandNum);
    RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, wocoActionIsWrite);
    WOCO::ECommand wocoCommand = (WOCO::ECommand)wocoCommandNum;
    Serial << "WOCO: " << wocoCommandNum << ", " << (uint16_t)m_pWOCO << endl;

    switch (wocoCommand)
    {
    case WOCO::ECommand::AliveCheck:
      m_pWOCO = WOCO_AliveCheck::CreateReadRequest ();
      break;

    case WOCO::ECommand::DeviceName:
      m_pWOCO = WOCO_DeviceName::CreateReadRequest ();
      break;

    case WOCO::ECommand::DeviceType:
      m_pWOCO = WOCO_DeviceType::CreateReadRequest ();
      break;

    case WOCO::ECommand::DigitalIOMode:
      RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, pinNumber);
      if (wocoActionIsWrite)
      {
        RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, pinMode);
        m_pWOCO = WOCO_DigitalIOMode::CreateWriteRequest (pinNumber, pinMode);
      }
      else
        m_pWOCO = WOCO_DigitalIOMode::CreateReadRequest (pinNumber);
      break;

    case WOCO::ECommand::DigitalIOState:
      RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, pinNumber);
      if (wocoActionIsWrite)
      {
        RingBuffer_GetValueAndMovePtr (m_CommandBuffer, sizeof (m_CommandBuffer), pCommandBufferCurrentPos, pinState);
        m_pWOCO = WOCO_DigitalIOState::CreateWriteRequest (pinNumber, pinState);
      }
      else
        m_pWOCO = WOCO_DigitalIOState::CreateReadRequest (pinNumber);
      break;

    default:
      Serial << "unsupported WOCO" << endl;
      break;
    }

    Serial << "WOCO: " << (uint16_t)m_pWOCO << endl;

    if (m_pWOCO != nullptr
    &&  !m_pTOBAController->get_ExistsRequest ())
    {
      result = m_pTOBAController->CreateRequest (workerDeviceId, m_pWOCO);
      Serial << F("CreateRequest(): ") << TOBA::GetResultText (result) << endl;

      // Send the request.
      if (m_pTOBAController->get_ExistsRequest ())
      {
        result = m_pTOBAController->SendRequest ();
        Serial << F("SendRequest(): ") << TOBA::GetResultText (result) << endl;
      }
    }
  }

  // Read data from the stored stream and write it into a buffer.
  result = m_pTOBAController->ReadData ();
  Serial << F("ReadData(): ") << TOBA::GetResultText (result) << endl;

  // Analyze the read data to find a request message.
  // This may only be done, if no active request, work, or reply exists; otherwise, an active request will be overwritten.
  if (m_pTOBAController->get_ExistsData ())
  {
    result = m_pTOBAController->AnalyzeData ();
    Serial << F("AnalyzeData(): ") << TOBA::GetResultText (result) << endl;
  }

  // Analyse the received reply to retrieve a worker reply.
  if (m_pTOBAController->get_ExistsReply ())
  {
    result = m_pTOBAController->AnalyzeReply (m_pWORE);
    Serial << F("AnalyzeReply(): ") << TOBA::GetResultText (result) << endl;
  }

  Serial << F("Sleeping ") << c_Sleep_ms << " ms." << endl << endl;
  delay (c_Sleep_ms);
}
