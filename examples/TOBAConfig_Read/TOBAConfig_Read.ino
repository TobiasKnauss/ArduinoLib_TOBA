#include <Streaming.h>

#include <TOBADeviceConfig.h>

const uint16_t c_EepromOffset = 16;

void setup ()
{
  Serial.begin (115200);
  delay (2000);

  TOBADeviceConfig* pTOBADeviceConfig = nullptr;
  EResult result = TOBADeviceConfig::Create (c_EepromOffset, pTOBADeviceConfig);
  Serial << F("TOBADeviceConfig.Create(..eepromAddress..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBADeviceConfig->Print ();

  UCOPConfig* pUCOPConfig = nullptr;
  result = UCOPConfig::Create (pTOBADeviceConfig->get_EepromAddress_UCOPConfig (), pUCOPConfig);
  Serial << "UCOPConfig.Create(..eepromAddress..), Result: " << (int)result << " = " << UCOP::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pUCOPConfig->Print ();
}

void loop ()
{
}
