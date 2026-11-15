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

const uint16_t c_EepromAddr_TOBADeviceConfig = 16;

WOCO* m_pWOCO = nullptr;
WOCO* m_pWORE = nullptr;
TOBADeviceConfig* m_pTOBADeviceConfig = nullptr;
TOBAController*   m_pTOBAController   = nullptr;

bool m_IsCommandComplete = false;
BytesBuffer* m_pCommandBuffer = nullptr;

uint32_t loopExec = 0;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (115200);
  Serial1.begin (9600);
  delay (2000);

  bool isOK = BytesBuffer::Create (20, 0xFF, false, m_pCommandBuffer);
  Serial << F("ByteBuffer.Create(): ") << isOK << endl;

  result = TOBADeviceConfig::Create (c_EepromAddr_TOBADeviceConfig, m_pTOBADeviceConfig);
  Serial << F("TOBADeviceConfig.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  result = TOBAController::Create (&Serial1, nullptr, (TOBAControllerConfig*)m_pTOBADeviceConfig, m_pTOBAController);
  Serial << F("TOBAController.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  Serial << endl << F("TOBA Config:") << endl;
  m_pTOBAController->get_Config ()->Print ();
  Serial << endl << F("UCOP Config:") << endl;
  m_pTOBAController->get_UCOPConfig ()->Print ();
  Serial << endl;
}

//--------------------------------------------------------------------
void loop ()
{
  if (m_pTOBAController == nullptr)
  {
    Serial << "Setup failed!" << endl;
    return;
  }

  loopExec++;
  EResult result;

  if (Serial.available ())
  {
    Serial << loopExec << F(": Serial available: ") << Serial.available () << endl;

    // Receive all available data
    bool isOK = true;
    while (Serial.available () > 0)
    {
      uint8_t value = Serial.read ();
      m_IsCommandComplete = value == 0xFF;
      if (m_IsCommandComplete)
        break;
      isOK &= m_pCommandBuffer->WriteValueAndMovePtr (value);
    }
    if (!isOK)
    {
      m_pCommandBuffer->Clear ();
      m_pCommandBuffer->SetWritePointer (0);
    }
  }

  if (m_pWOCO == nullptr
  &&  m_IsCommandComplete)
  {
    m_pCommandBuffer->SetWritePointer (0);
    m_pCommandBuffer->SetReadPointer (0);
    Serial << loopExec << F(": CommandBuffer: bytes used = ") << m_pCommandBuffer->get_CurrentWriteAddress () << endl;
    m_pCommandBuffer->Print (&Serial, true);

    uint32_t workerDeviceId     = 0;
    uint16_t wocoCommandNum     = 0;
    bool     wocoActionIsWrite  = false;
    uint8_t  pinNumber          = 0;
    uint8_t  pinMode            = 0;
    bool     pinState           = false;
    bool isOK = m_pCommandBuffer->ReadValueAndMovePtr (workerDeviceId, true);
        isOK &= m_pCommandBuffer->ReadValueAndMovePtr (wocoCommandNum, true);
        isOK &= m_pCommandBuffer->ReadValueAndMovePtr (wocoActionIsWrite);
    WOCO::ECommand wocoCommand = (WOCO::ECommand)wocoCommandNum;
    Serial << loopExec << F(": CommandBuffer.ReadValueAndMovePtr() ") << isOK << ", WOCO: " << wocoCommandNum << endl;

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
      m_pCommandBuffer->ReadValueAndMovePtr (pinNumber);
      if (wocoActionIsWrite)
      {
        m_pCommandBuffer->ReadValueAndMovePtr (pinMode);
        m_pWOCO = WOCO_DigitalIOMode::CreateWriteRequest (pinNumber, pinMode);
      }
      else
        m_pWOCO = WOCO_DigitalIOMode::CreateReadRequest (pinNumber);
      break;

    case WOCO::ECommand::DigitalIOState:
      m_pCommandBuffer->ReadValueAndMovePtr (pinNumber);
      if (wocoActionIsWrite)
      {
        m_pCommandBuffer->ReadValueAndMovePtr (pinState);
        m_pWOCO = WOCO_DigitalIOState::CreateWriteRequest (pinNumber, pinState);
      }
      else
        m_pWOCO = WOCO_DigitalIOState::CreateReadRequest (pinNumber);
      break;

    default:
      Serial << loopExec << F(": unsupported WOCO") << endl;
      break;
    }

    m_pCommandBuffer->Clear ();

    if (m_pWOCO != nullptr)
    {
      Serial << "WOCO: " << endl;
      m_pWOCO->Print ();
    }

    if (m_pWOCO != nullptr
    &&  !m_pTOBAController->get_ExistsRequest ())
    {
      result = m_pTOBAController->CreateRequest (workerDeviceId, m_pWOCO);
      Serial << loopExec << F(": CreateRequest(): ") << TOBA::GetResultText (result) << endl;

      // Send the request.
      if (m_pTOBAController->get_ExistsRequest ())
      {
        result = m_pTOBAController->SendRequest ();
        Serial << loopExec << F(": SendRequest(): ") << TOBA::GetResultText (result) << endl;
      }
    }
  }

  // Read data from the stored stream and write it into a buffer.
  result = m_pTOBAController->ReadData ();
  Serial << loopExec << F(": ReadData(): ") << TOBA::GetResultText (result) << endl;

  // Analyze the read data to find a request message.
  // This may only be done, if no active request, work, or reply exists; otherwise, an active request will be overwritten.
  if (m_pTOBAController->get_ExistsData ())
  {
    result = m_pTOBAController->AnalyzeData ();
    Serial << loopExec << F(": AnalyzeData(): ") << TOBA::GetResultText (result) << endl;
  }

  // Analyse the received reply to retrieve a worker reply.
  if (m_pTOBAController->get_ExistsReply ())
  {
    result = m_pTOBAController->AnalyzeReply (m_pWORE);
    Serial << loopExec << F(": AnalyzeReply(): ") << TOBA::GetResultText (result) << endl;
  }

  Serial << loopExec << F(": Sleeping ") << c_Sleep_ms << " ms." << endl << endl;
  delay (c_Sleep_ms);
}
