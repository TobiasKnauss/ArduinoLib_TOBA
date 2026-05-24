#include <Arduino.h>
#include <Streaming.h>
#include <UCOP.h>
#include <UCOPConfig.h>
#include "TOBAWorker.h"
#include "TOBAConfig.h"
#include "TOBAWorker_CustomIO.h"
#include "TOBAConfig_CustomIO.h"

const uint16_t c_EepromAddr_WorkerConfig = 0;
const uint16_t c_EepromAddr_UcopConfig = 42;

UCOPConfig*           m_pUCOPConfig           = nullptr;
UCOP*                 m_pUCOP                 = nullptr;
TOBAConfig*           m_pTOBAConfig           = nullptr;
TOBAWorker*           m_pTOBAWorker           = nullptr;
TOBAConfig_CustomIO*  m_pTOBAConfig_CustomIO  = nullptr;
TOBAWorker_CustomIO*  m_pTOBAWorker_CustomIO  = nullptr;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  result = UCOPConfig::Create (true, true, false, 0x63691401, UCOP::EChecksumType::CRC8, m_pUCOPConfig);
  result = UCOP::Create (m_pUCOPConfig, m_pUCOP);

  char workerName[] = "W1"; 
  result = TOBAConfig::Create (80, 50, 20, workerName, strlen (workerName), 0, m_pTOBAConfig);
  result = TOBAConfig::Create (0, m_pTOBAConfig);
  result = TOBAWorker::Create (&Serial1, m_pUCOP, m_pTOBAConfig, m_pTOBAWorker);

  uint8_t ioPins[] = {1,2,3,4};
  result = TOBAConfig_CustomIO::Create (80, 50, 20, workerName, strlen (workerName), 0, 4, ioPins, m_pTOBAConfig_CustomIO);
  result = TOBAWorker_CustomIO::Create (&Serial1, m_pUCOP, m_pTOBAConfig_CustomIO, m_pTOBAWorker_CustomIO);
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;

  result = m_pTOBAWorker         ->ReadData ();
  result = m_pTOBAWorker_CustomIO->ReadData ();
  Serial.println (TOBAWorker::GetResultText (result));

  result = m_pTOBAWorker         ->AnalyzeData ();
  result = m_pTOBAWorker_CustomIO->AnalyzeData ();
  Serial.println (TOBAWorker::GetResultText (result));

  result = m_pTOBAWorker         ->AnalyzeRequest ();
  result = m_pTOBAWorker_CustomIO->AnalyzeRequest ();
  Serial.println (TOBAWorker::GetResultText (result));
  
  result = m_pTOBAWorker         ->Work ();
  result = m_pTOBAWorker_CustomIO->Work ();
  Serial.println (TOBAWorker::GetResultText (result));

  result = m_pTOBAWorker         ->SendReply ();
  result = m_pTOBAWorker_CustomIO->SendReply ();
  Serial.println (TOBAWorker::GetResultText (result));


  Serial.println ("Idle.");
  delay (1000);
}
