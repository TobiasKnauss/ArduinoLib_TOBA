#include <Streaming.h>

#include <TOBADeviceConfig.h>
#include <TOBAControllerConfig.h>

const uint16_t c_EepromOffset = 16;

const uint16_t m_ReceiveBufferSize        = 200;
const uint16_t m_SendBufferSize           = 50;
const uint16_t m_PayloadBuffersSize       = 20;
const char     m_DeviceName[]             = "Controller #1";
const uint8_t  m_DeviceNameLength         = sizeof (m_DeviceName);
const uint16_t m_EepromAddress_UCOPConfig = 0;

void setup ()
{
  Serial.begin (115200);
  delay (2000);

  TOBAControllerConfig* pTOBAControllerConfig1 = nullptr;
  EResult result = TOBAControllerConfig::Create ( m_ReceiveBufferSize,
                                                  m_SendBufferSize,
                                                  m_PayloadBuffersSize,
                                                  m_DeviceName,
                                                  m_DeviceNameLength,
                                                  m_EepromAddress_UCOPConfig,
                                                  pTOBAControllerConfig1);
  Serial << F("TOBAControllerConfig.Create(..data..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  pTOBAControllerConfig1->Print ();

  result = pTOBAControllerConfig1->WriteToEEPROM (c_EepromOffset);
  Serial << F("TOBADeviceConfig.WriteToEEPROM, Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  TOBADeviceConfig* pTOBADeviceConfig2 = nullptr;
  result = TOBADeviceConfig::Create (c_EepromOffset, pTOBADeviceConfig2);
  Serial << F("TOBADeviceConfig.Create(..eepromAddress..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBADeviceConfig2->Print ();
}

void loop ()
{
}
