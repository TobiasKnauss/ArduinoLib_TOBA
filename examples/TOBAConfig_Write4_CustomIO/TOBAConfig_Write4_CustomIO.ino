#include <Streaming.h>

#include <TOBADeviceConfig.h>
#include <TOBAWorkerConfig_CustomIO.h>

const bool     c_WriteConfig = false;  // <--- Set this flag to TRUE if the config has to be changed.
const uint16_t c_EepromOffset = 00;

const uint16_t m_ReceiveBufferSize        = 80;
const uint16_t m_SendBufferSize           = 50;
const uint16_t m_PayloadBuffersSize       = 20;
const char     m_DeviceName[]             = "CustomIO Worker #1";
const uint8_t  m_DeviceNameLength         = sizeof (m_DeviceName);
const uint16_t m_EepromAddress_UCOPConfig = 60;
const uint8_t  m_IOPins[]                 = { 2,3,4,5,6,7,8,9 };

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  TOBAWorkerConfig_CustomIO* pTOBAWorkerConfig1 = nullptr;
  EResult result = TOBAWorkerConfig_CustomIO::Create (m_ReceiveBufferSize,
                                                      m_SendBufferSize,
                                                      m_PayloadBuffersSize,
                                                      m_DeviceName,
                                                      m_DeviceNameLength,
                                                      m_EepromAddress_UCOPConfig,
                                                      sizeof (m_IOPins),
                                                      m_IOPins,
                                                      pTOBAWorkerConfig1);
  Serial << F("TOBAWorkerConfig_CustomIO.Create(..data..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
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
