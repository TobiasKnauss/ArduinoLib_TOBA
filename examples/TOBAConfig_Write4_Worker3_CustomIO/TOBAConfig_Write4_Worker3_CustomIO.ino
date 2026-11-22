#include <Streaming.h>

#include <TOBADeviceConfig.h>
#include <TOBAWorkerConfig_CustomIO.h>

const uint16_t c_EepromAddr_TOBADeviceConfig = 16;

const uint16_t c_ReceiveBufferSize        = 80;
const uint16_t c_SendBufferSize           = 50;
const uint16_t c_PayloadBuffersSize       = 20;
const char     c_DeviceName[]             = "Worker #3: CustomIO";
const uint8_t  c_DeviceNameLength         = sizeof (c_DeviceName);
const uint16_t c_EepromAddress_UCOPConfig = 0;
const uint8_t  c_IOPins[]                 = { 2,3,4,5,6,7,8,9 };

HardwareSerial* m_pLogStream = &Serial;

//--------------------------------------------------------------------
void setup ()
{
  m_pLogStream->begin (115200);
  delay (2000);

  TOBAWorkerConfig_CustomIO* pTOBAWorkerConfig1 = nullptr;
  EResult result = TOBAWorkerConfig_CustomIO::Create (c_ReceiveBufferSize,
                                                      c_SendBufferSize,
                                                      c_PayloadBuffersSize,
                                                      c_DeviceName,
                                                      c_DeviceNameLength,
                                                      c_EepromAddress_UCOPConfig,
                                                      sizeof (c_IOPins),
                                                      c_IOPins,
                                                      pTOBAWorkerConfig1);
  *m_pLogStream << F("TOBAWorkerConfig_CustomIO.Create(..data..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
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
