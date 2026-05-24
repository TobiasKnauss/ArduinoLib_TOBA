#include <Streaming.h>

#include <TOBAWorker.h>
#include <TOBAConfig.h>

const uint16_t c_EepromOffset = 0;

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  TOBAConfig* pTOBAConfig = nullptr;
  EResult result = TOBAConfig::Create (c_EepromOffset, pTOBAConfig);
  Serial << "TOBAConfig.Create(..eepromAddress..), Result: " << (int)result << " = " << TOBAWorker::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBAConfig->Print ();
  }

void loop ()
{
}
