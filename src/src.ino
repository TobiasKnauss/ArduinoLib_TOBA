#include <Arduino.h>
#include <Streaming.h>
#include <UCOP.h>
#include <UCOPConfig.h>
#include <WOCO_DeviceName.h>

#include "TOBAWorker.h"
#include "TOBAWorkerConfig.h"
#include "TOBAWorker_CustomIO.h"
#include "TOBAWorkerConfig_CustomIO.h"
#include "TOBAController.h"
#include "TOBAControllerConfig.h"

const uint16_t c_EepromAddr_WorkerConfig = 0;
const uint16_t c_EepromAddr_UcopConfig = 60;

UCOPConfig*                 m_pUCOPConfig                 = nullptr;
UCOP*                       m_pUCOP                       = nullptr;

TOBADeviceConfig*           m_pTOBADeviceConfig           = nullptr;

TOBAWorkerConfig*           m_pTOBAWorkerConfig           = nullptr;
TOBAWorker*                 m_pTOBAWorker                 = nullptr;

TOBAWorkerConfig_CustomIO*  m_pTOBAWorkerConfig_CustomIO  = nullptr;
TOBAWorker*                 m_pTOBAWorker_CustomIO        = nullptr;

TOBAControllerConfig*       m_pTOBAControllerConfig       = nullptr;
TOBAController*             m_pTOBAController             = nullptr;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  result = UCOPConfig::Create (true, true, false, 0x63691401, UCOP::EChecksumType::CRC8, m_pUCOPConfig);
  result = UCOP::Create (m_pUCOPConfig, m_pUCOP);
  Serial.println (TOBA::GetResultText (result));

  result = TOBADeviceConfig::Create (0, m_pTOBADeviceConfig);

  char workerName[] = "W1";
  result = TOBAWorkerConfig::Create (80, 50, 20, workerName, strlen (workerName), 0, m_pTOBAWorkerConfig);
  result = TOBAWorker::Create (&Serial1, m_pUCOP, m_pTOBAWorkerConfig, m_pTOBAWorker);

  uint8_t ioPins[] = {1,2,3,4};
  result = TOBAWorkerConfig_CustomIO::Create (80, 50, 20, workerName, strlen (workerName), 0, 4, ioPins, m_pTOBAWorkerConfig_CustomIO);
  result = TOBAWorker::Create (&Serial1, m_pUCOP, m_pTOBAWorkerConfig_CustomIO, m_pTOBAWorker_CustomIO);

  char controllerName[] = "C1";
  result = TOBAControllerConfig::Create (80, 50, 20, controllerName, strlen (controllerName), 0, m_pTOBAControllerConfig);
  result = TOBAController::Create (&Serial1, m_pUCOP, m_pTOBAControllerConfig, m_pTOBAController);
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;

  result = m_pTOBAWorker         ->ReadData ();
  result = m_pTOBAWorker_CustomIO->ReadData ();

  result = m_pTOBAWorker         ->AnalyzeData ();
  result = m_pTOBAWorker_CustomIO->AnalyzeData ();

  result = m_pTOBAWorker         ->AnalyzeRequest ();
  result = m_pTOBAWorker_CustomIO->AnalyzeRequest ();

  result = m_pTOBAWorker         ->Work ();
  result = m_pTOBAWorker_CustomIO->Work ();

  result = m_pTOBAWorker         ->SendReply ();
  result = m_pTOBAWorker_CustomIO->SendReply ();

  Serial.println (TOBA::GetResultText (result));

  uint32_t workerDeviceId = 0;
  WOCO* pWOCO_DeviceName = WOCO_DeviceName::CreateReadRequest ();
  WOCO* pWORE_DeviceName = nullptr;

  result = m_pTOBAController->CreateRequest (workerDeviceId, pWOCO_DeviceName);

  result = m_pTOBAController->SendRequest ();

  result = m_pTOBAController->ReadData ();

  result = m_pTOBAController->AnalyzeData ();

  result = m_pTOBAController->AnalyzeReply (pWORE_DeviceName);

  Serial.println (TOBA::GetResultText (result));
}
