#include <Streaming.h>

#include <TOBADeviceConfig.h>

const uint16_t c_EepromOffset = 0;

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  TOBADeviceConfig* pTOBADeviceConfig = nullptr;
  EResult result = TOBADeviceConfig::Create (c_EepromOffset, pTOBADeviceConfig);
  Serial << F("TOBADeviceConfig.Create(..eepromAddress..), Result: ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBADeviceConfig->Print ();
  }

void loop ()
{
}
