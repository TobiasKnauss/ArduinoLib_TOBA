#include <Streaming.h>

#include <TOBADeviceConfig.h>
#include <TOBAWorkerConfig.h>

const bool     c_WriteConfig = false;  // <--- Set this flag to TRUE if the config has to be changed.
const uint16_t c_EepromOffset = 00;

const uint16_t m_ReceiveBufferSize        = 80;
const uint16_t m_SendBufferSize           = 50;
const uint16_t m_PayloadBuffersSize       = 20;
const char     m_DeviceName[]             = "Basic Worker #1";
const uint8_t  m_DeviceNameLength         = sizeof (m_DeviceName);
const uint16_t m_EepromAddress_UCOPConfig = 60;

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  TOBAWorkerConfig* pTOBAWorkerConfig1 = nullptr;
  EResult result = TOBAWorkerConfig::Create ( m_ReceiveBufferSize,
                                              m_SendBufferSize,
                                              m_PayloadBuffersSize,
                                              m_DeviceName,
                                              m_DeviceNameLength,
                                              m_EepromAddress_UCOPConfig,
                                              pTOBAWorkerConfig1);
  Serial << F("TOBAWorkerConfig.Create(..data..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  pTOBAWorkerConfig1->Print ();

  result = pTOBAWorkerConfig1->WriteToEEPROM (c_EepromOffset);
  Serial << F("TOBADeviceConfig.WriteToEEPROM, Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  TOBADeviceConfig* pTOBADeviceConfig2 = nullptr;
  result = TOBADeviceConfig::Create (c_EepromOffset, pTOBADeviceConfig2);
  Serial << F("TOBAWorkerConfig.Create(..eepromAddress..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBADeviceConfig2->Print ();
}

void loop ()
{
}
