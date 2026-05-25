#include <Arduino.h>
#include <Streaming.h>

#include "TOBAWorker.h"
#include "TOBAConfig.h"

const uint16_t c_Sleep_ms = 1000;

const uint16_t c_EepromAddr_WorkerConfig = 0;

TOBAConfig* m_pTOBAConfig = nullptr;
TOBAWorker* m_pTOBAWorker = nullptr;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  result = TOBAConfig::Create (0, m_pTOBAConfig);
  Serial << "TOBAConfig.Create, Result: " << (int)result << " = " << TOBAWorker::GetResultText (result) << endl;

  result = TOBAWorker::Create (&Serial1, nullptr, m_pTOBAConfig, m_pTOBAWorker);
  Serial << "TOBAWorker.Create, Result: " << (int)result << " = " << TOBAWorker::GetResultText (result) << endl;
}

//--------------------------------------------------------------------
void loop ()
{
  if (m_pTOBAWorker == nullptr)
    return;

  EResult result;

  result = m_pTOBAWorker->ReadData ();
  Serial.println (TOBAWorker::GetResultText (result));
  Serial << "Data avilable: " << m_pTOBAWorker->get_IsDataAvailable () << endl;;

  result = m_pTOBAWorker->AnalyzeData ();
  Serial.println (TOBAWorker::GetResultText (result));

  result = m_pTOBAWorker->AnalyzeRequest ();
  Serial.println (TOBAWorker::GetResultText (result));

  result = m_pTOBAWorker->Work ();
  Serial.println (TOBAWorker::GetResultText (result));

  result = m_pTOBAWorker->SendReply ();
  Serial.println (TOBAWorker::GetResultText (result));

  Serial << "Sleeping " << c_Sleep_ms << " ms." << endl << endl;
  delay (c_Sleep_ms);
}
