#include <Arduino.h>
#include <Streaming.h>

#include "TOBA_Worker.h"

TOBA_Worker* m_pTobaWorker = nullptr;


//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  m_pTobaWorker = new TOBA_Worker (&Serial1, 80, 50, 20, "W1", 2, result);
  
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;

  result = m_pTobaWorker->ReadData ();
  Serial.println (TOBA_Worker::GetResultText (result));

  result = m_pTobaWorker->AnalyzeData ();
  Serial.println (TOBA_Worker::GetResultText (result));

  result = m_pTobaWorker->AnalyzeRequest ();
  Serial.println (TOBA_Worker::GetResultText (result));
  
  result = m_pTobaWorker->Work ();
  Serial.println (TOBA_Worker::GetResultText (result));

  result = m_pTobaWorker->SendReply ();
  Serial.println (TOBA_Worker::GetResultText (result));


  Serial.println ("Idle.");
  delay (1000);
}
