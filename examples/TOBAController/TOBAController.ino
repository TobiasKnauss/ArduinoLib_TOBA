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

const uint8_t  c_PinNumber_SendEnable     = 10;
const uint8_t  c_PinNumber_ReceiveDisable = 22;

const uint16_t c_EepromAddr_TOBADeviceConfig = 16;

HardwareSerial* m_pLogStream  = &Serial;
HardwareSerial* m_pCommStream = &Serial1;
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
  // MCU startup signal
  int pinNumber_LED = 9;
  pinMode (pinNumber_LED, OUTPUT);
  for (int blink = 0; blink < 5; blink++)
  {
    digitalWrite (pinNumber_LED, 1);
    delay (100);
    digitalWrite (pinNumber_LED, 0);
    delay (500);
  }

  // start of main program
  EResult result;

  m_pLogStream->begin (115200);
  m_pCommStream->begin (9600);
  delay (2000);

  pinMode (c_PinNumber_SendEnable,     OUTPUT);
  pinMode (c_PinNumber_ReceiveDisable, OUTPUT);
  digitalWrite (c_PinNumber_SendEnable,     LOW);
  digitalWrite (c_PinNumber_ReceiveDisable, LOW);

  bool isOK = BytesBuffer::Create (20, 0xFF, false, m_pCommandBuffer);
  *m_pLogStream << F("ByteBuffer.Create(): ") << isOK << endl;

  result = TOBADeviceConfig::Create (c_EepromAddr_TOBADeviceConfig, m_pTOBADeviceConfig);
  *m_pLogStream << F("TOBADeviceConfig.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  result = TOBAController::Create (m_pCommStream, nullptr, (TOBAControllerConfig*)m_pTOBADeviceConfig, m_pTOBAController);
  *m_pLogStream << F("TOBAController.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  *m_pLogStream << endl << F("TOBA Config:") << endl;
  m_pTOBAController->get_Config ()->Print (m_pLogStream);
  *m_pLogStream << endl << F("UCOP Config:") << endl;
  m_pTOBAController->get_UCOPConfig ()->Print (m_pLogStream);
  *m_pLogStream << endl;

  m_pTOBAController->set_DebugOutput (m_pLogStream);
}

//--------------------------------------------------------------------
void loop ()
{
  if (m_pTOBAController == nullptr)
  {
    *m_pLogStream << "Setup failed!" << endl;
    return;
  }

  loopExec++;
  EResult result;

  if (m_pLogStream->available ())
  {
    *m_pLogStream << loopExec << F(": LogStream available: ") << m_pLogStream->available () << endl;

    // Receive all available data
    bool isOK = true;
    while (m_pLogStream->available () > 0)
    {
      uint8_t value = m_pLogStream->read ();
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
    m_IsCommandComplete = false;

    *m_pLogStream << loopExec << F(": CommandBuffer: bytes used = ") << m_pCommandBuffer->get_CurrentWriteAddress () << endl;
    m_pCommandBuffer->Print (m_pLogStream, true);

    m_pCommandBuffer->SetWritePointer (0);
    m_pCommandBuffer->SetReadPointer (0);

    // Command:  WWWWCCAxxxx
    // WWWW = Worker Device ID (4 bytes)
    // CC   = Command Number (2 bytes)
    // A    = Action (1 byte)
    // xxxx = optional command-specific parameters

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
    *m_pLogStream << loopExec << F(": CommandBuffer.ReadValueAndMovePtr() ") << (isOK ? "OK" : "FAIL") << ", WOCO Cmd Num: " << wocoCommandNum << endl;

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
      *m_pLogStream << loopExec << F(": unsupported WOCO") << endl;
      break;
    }

    m_pCommandBuffer->Clear ();

    if (m_pWOCO != nullptr)
    {
      *m_pLogStream << "WOCO: " << endl;
      m_pWOCO->Print (m_pLogStream);
    }

    if (m_pWOCO != nullptr
    &&  !m_pTOBAController->get_ExistsRequest ())
    {
      result = m_pTOBAController->CreateRequest (workerDeviceId, m_pWOCO);
      *m_pLogStream << loopExec << F(": CreateRequest(): ") << TOBA::GetResultText (result) << endl;

      // Send the request.
      if (m_pTOBAController->get_ExistsRequest ())
      {
        digitalWrite (c_PinNumber_ReceiveDisable, HIGH);
        digitalWrite (c_PinNumber_SendEnable,     HIGH);

        result = m_pTOBAController->SendRequest ();
        *m_pLogStream << loopExec << F(": SendRequest(): ") << TOBA::GetResultText (result) << endl;

        digitalWrite (c_PinNumber_SendEnable, LOW);
        digitalWrite (c_PinNumber_ReceiveDisable, LOW);
      }
    }
  }

  // Read data from the stored stream and write it into a buffer.
  result = m_pTOBAController->ReadData ();
  *m_pLogStream << loopExec << F(": ReadData(): ") << TOBA::GetResultText (result) << endl;

  // Analyze the read data to find a request message.
  // This may only be done, if no active request, work, or reply exists; otherwise, an active request will be overwritten.
  if (m_pTOBAController->get_ExistsData ())
  {
    result = m_pTOBAController->AnalyzeData ();
    *m_pLogStream << loopExec << F(": AnalyzeData(): ") << TOBA::GetResultText (result) << endl;
  }

  // Analyse the received reply to retrieve a worker reply.
  if (m_pTOBAController->get_ExistsReply ())
  {
    result = m_pTOBAController->AnalyzeReply (m_pWORE);
    *m_pLogStream << loopExec << F(": AnalyzeReply(): ") << TOBA::GetResultText (result) << endl;
  }

  *m_pLogStream << loopExec << F(": Sleeping ") << c_Sleep_ms << " ms." << endl << endl;
  delay (c_Sleep_ms);
}
