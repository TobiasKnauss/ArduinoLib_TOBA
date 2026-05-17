#include <Arduino.h>
#include <Streaming.h>

#include "TOBA_Worker.h"

const uint16_t c_EepromAddr_WorkerConfig = 0;
const uint16_t c_EepromAddr_UcopConfig = 42;

UCOP*         m_pUCOP       = nullptr;
TOBA_Worker*  m_pTobaWorker = nullptr;


//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  m_pUCOP = new UCOP (true, true, false, 0x63691401, UCOP::EChecksumType::CRC8, result);
  char workerName[] = "W1"; 
  m_pTobaWorker = new TOBA_Worker (&Serial1, 80, 50, 20, workerName, strlen (workerName), m_pUCOP, result);
  
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
