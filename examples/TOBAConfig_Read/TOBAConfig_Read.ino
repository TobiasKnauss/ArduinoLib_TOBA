#include <Streaming.h>

#include <TOBAWorker_Basic.h>
#include <TOBAConfig_Basic.h>

const uint16_t c_EepromOffset = 0;

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  TOBAConfig_Basic* pTOBAConfig = nullptr;
  EResult result = TOBAConfig_Basic::Create (c_EepromOffset, pTOBAConfig);
  Serial << "TOBAConfig_Basic.Create(..eepromAddress..), Result: " << (int)result << " = " << TOBAWorker_Basic::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBAConfig->Print ();
  }

void loop ()
{
}
