#include <Streaming.h>

#include <TOBADeviceConfig.h>

const uint16_t c_EepromAddr_TOBADeviceConfig = 16;

HardwareSerial* m_pLogStream = &Serial;

//--------------------------------------------------------------------
void setup ()
{
  m_pLogStream->begin (115200);
  delay (2000);

  TOBADeviceConfig* pTOBADeviceConfig = nullptr;
  EResult result = TOBADeviceConfig::Create (c_EepromAddr_TOBADeviceConfig, pTOBADeviceConfig);
  *m_pLogStream << F("TOBADeviceConfig.Create(..eepromAddress..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBADeviceConfig->Print (m_pLogStream);

  UCOPConfig* pUCOPConfig = nullptr;
  result = UCOPConfig::Create (pTOBADeviceConfig->get_EepromAddress_UCOPConfig (), pUCOPConfig);
  *m_pLogStream << "UCOPConfig.Create(..eepromAddress..), Result: " << (int)result << " = " << UCOP::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pUCOPConfig->Print (m_pLogStream);
}

//--------------------------------------------------------------------
void loop ()
{
}
