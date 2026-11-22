#include <Streaming.h>

#include <TOBADeviceConfig.h>
#include <TOBAWorkerConfig.h>

const uint16_t c_EepromAddr_TOBADeviceConfig = 16;

const uint16_t c_ReceiveBufferSize        = 80;
const uint16_t c_SendBufferSize           = 50;
const uint16_t c_PayloadBuffersSize       = 20;
const char     c_DeviceName[]             = "Worker #1: Basic";
const uint8_t  c_DeviceNameLength         = sizeof (c_DeviceName);
const uint16_t c_EepromAddress_UCOPConfig = 0;

HardwareSerial* m_pLogStream = &Serial;

//--------------------------------------------------------------------
void setup ()
{
  m_pLogStream->begin (115200);
  delay (2000);

  TOBAWorkerConfig* pTOBAWorkerConfig1 = nullptr;
  EResult result = TOBAWorkerConfig::Create ( c_ReceiveBufferSize,
                                              c_SendBufferSize,
                                              c_PayloadBuffersSize,
                                              c_DeviceName,
                                              c_DeviceNameLength,
                                              c_EepromAddress_UCOPConfig,
                                              pTOBAWorkerConfig1);
  *m_pLogStream << F("TOBAWorkerConfig.Create(..data..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  pTOBAWorkerConfig1->Print (m_pLogStream);

  result = pTOBAWorkerConfig1->WriteToEEPROM (c_EepromAddr_TOBADeviceConfig);
  *m_pLogStream << F("TOBADeviceConfig.WriteToEEPROM, Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  TOBADeviceConfig* pTOBADeviceConfig2 = nullptr;
  result = TOBADeviceConfig::Create (c_EepromAddr_TOBADeviceConfig, pTOBADeviceConfig2);
  *m_pLogStream << F("TOBADeviceConfig.Create(..eepromAddress..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBADeviceConfig2->Print (m_pLogStream);
}

//--------------------------------------------------------------------
void loop ()
{
}
