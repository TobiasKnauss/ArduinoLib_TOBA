#include <Arduino.h>
#include <Streaming.h>
#include <UCOP.h>
#include <TOBAWorker_Basic.h>
#include <TOBAConfig_Basic.h>

const uint16_t c_EepromAddr_WorkerConfig = 0;
const uint16_t c_EepromAddr_UcopConfig = 42;

UCOP*             m_pUCOP       = nullptr;
TOBAConfig_Basic* m_pTOBAConfig = nullptr;
TOBAWorker_Basic* m_pTOBAWorker = nullptr;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  m_pUCOP = new UCOP (true, true, false, 0x63691401, UCOP::EChecksumType::CRC8, result);
  char workerName[] = "W1"; 
  result = TOBAConfig_Basic::Create (80, 50, 20, workerName, strlen (workerName), 0, m_pTOBAConfig);
  result = TOBAWorker_Basic::Create (&Serial1, m_pUCOP, m_pTOBAConfig, m_pTOBAWorker);
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;

  result = m_pTOBAWorker->ReadData ();
  Serial.println (TOBAWorker_Basic::GetResultText (result));

  result = m_pTOBAWorker->AnalyzeData ();
  Serial.println (TOBAWorker_Basic::GetResultText (result));

  result = m_pTOBAWorker->AnalyzeRequest ();
  Serial.println (TOBAWorker_Basic::GetResultText (result));
  
  result = m_pTOBAWorker->Work ();
  Serial.println (TOBAWorker_Basic::GetResultText (result));

  result = m_pTOBAWorker->SendReply ();
  Serial.println (TOBAWorker_Basic::GetResultText (result));


  Serial.println ("Idle.");
  delay (1000);
}
