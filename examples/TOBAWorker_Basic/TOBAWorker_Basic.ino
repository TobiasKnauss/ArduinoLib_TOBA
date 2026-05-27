#include <Arduino.h>
#include <Streaming.h>

#include "TOBAWorker.h"
#include "TOBAWorkerConfig.h"

const uint16_t c_Sleep_ms = 1000;

const uint16_t c_EepromAddr_WorkerConfig = 0;

TOBADeviceConfig* m_pTOBADeviceConfig = nullptr;
TOBAWorker* m_pTOBAWorker = nullptr;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  result = TOBADeviceConfig::Create (0, m_pTOBADeviceConfig);
  Serial << F("TOBADeviceConfig.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  result = TOBAWorker::Create (&Serial1, nullptr, m_pTOBADeviceConfig, m_pTOBAWorker);
  Serial << F("TOBAWorker.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;
}

//--------------------------------------------------------------------
void loop ()
{
  if (m_pTOBAWorker == nullptr)
    return;

  EResult result;

  // Read data from the stored stream and write it into a buffer.
  result = m_pTOBAWorker->ReadData ();
  Serial << F("ReadData(): ") << TOBA::GetResultText (result) << endl;
  Serial << "Data exists: " << m_pTOBAWorker->get_ExistsData () << endl;;

  // Analyze the read data to find a request message.
  // This may only be done, if no active request, work, or reply exists; otherwise, an active request will be overwritten.
  if (m_pTOBAWorker->get_ExistsData ()
  &&  !m_pTOBAWorker->get_IsBusy ())
  {
    result = m_pTOBAWorker->AnalyzeData ();
    Serial << F("AnalyzeData(): ") << TOBA::GetResultText (result) << endl;
  }

  // Analyse the received request to retrieve a worker command.
  // It is sufficient to check if a request exists, because one was only created if the worker was not busy.
  if (m_pTOBAWorker->get_ExistsRequest ())
  {
    result = m_pTOBAWorker->AnalyzeRequest ();
    Serial << F("AnalyzeRequest(): ") << TOBA::GetResultText (result) << endl;
  }

  // Execute the received worker command and create a reply.
  // It usually is sufficient to check if a worker command exists, because one was only created if the worker was not busy.
  if (m_pTOBAWorker->get_ExistsWork ())
  {
    result = m_pTOBAWorker->Work ();
    Serial << F("Work(): ") << TOBA::GetResultText (result) << endl;
  }

  // Send the reply that was created during analysis or work.
  // It usually is sufficient to check if a reply exists, because one was only created if the worker has finished or a failure happened.
  if (m_pTOBAWorker->get_ExistsReply ())
  {
    result = m_pTOBAWorker->SendReply ();
    Serial << F("SendReply(): ") << TOBA::GetResultText (result) << endl;
  }

  Serial << F("Sleeping ") << c_Sleep_ms << " ms." << endl << endl;
  delay (c_Sleep_ms);
}
