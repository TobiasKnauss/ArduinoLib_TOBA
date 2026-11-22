#include <Arduino.h>
#include <Streaming.h>

#include "TOBAWorker.h"
#include "TOBAWorkerConfig.h"

const uint16_t c_Sleep_ms = 1000;

const uint8_t  c_PinNumber_SendEnable     = 10;
const uint8_t  c_PinNumber_ReceiveDisable = 22;

const uint16_t c_EepromAddr_TOBADeviceConfig = 16;

HardwareSerial* m_pLogStream  = &Serial;
HardwareSerial* m_pCommStream = &Serial1;
TOBADeviceConfig* m_pTOBADeviceConfig = nullptr;
TOBAWorker* m_pTOBAWorker = nullptr;

uint32_t loopExec = 0;

//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  m_pLogStream->begin (115200);
  m_pCommStream->begin (9600);
  delay (2000);

  pinMode (c_PinNumber_SendEnable,     OUTPUT);
  pinMode (c_PinNumber_ReceiveDisable, OUTPUT);
  digitalWrite (c_PinNumber_SendEnable,     LOW);
  digitalWrite (c_PinNumber_ReceiveDisable, LOW);

  result = TOBADeviceConfig::Create (c_EepromAddr_TOBADeviceConfig, m_pTOBADeviceConfig);
  *m_pLogStream << F("TOBADeviceConfig.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  result = TOBAWorker::Create (m_pCommStream, nullptr, (TOBAWorkerConfig*)m_pTOBADeviceConfig, m_pTOBAWorker);
  *m_pLogStream << F("TOBAWorker.Create(): ") << (int)result << " = " << TOBA::GetResultText (result) << endl;

  *m_pLogStream << endl << F("TOBA Config:") << endl;
  m_pTOBAWorker->get_Config ()->Print (m_pLogStream);
  *m_pLogStream << endl << F("UCOP Config:") << endl;
  m_pTOBAWorker->get_UCOPConfig ()->Print (m_pLogStream);
  *m_pLogStream << endl;

  m_pTOBAWorker->set_DebugOutput (m_pLogStream);
}

//--------------------------------------------------------------------
void loop ()
{
  if (m_pTOBAWorker == nullptr)
  {
    *m_pLogStream << "Setup failed!" << endl;
    return;
  }

  loopExec++;
  EResult result;

  // Read data from the stored stream and write it into a buffer.
  result = m_pTOBAWorker->ReadData ();
  *m_pLogStream << loopExec << F(": ReadData(): ") << TOBA::GetResultText (result) << endl;

  // Analyze the read data to find a request message.
  // This may only be done, if no active request, work, or reply exists; otherwise, an active request will be overwritten.
  if (m_pTOBAWorker->get_ExistsData ())
  {
    *m_pLogStream << loopExec << F(": Data exists: ") << m_pTOBAWorker->get_ExistsData () << endl;
    if (!m_pTOBAWorker->get_IsBusy ())
    {
      result = m_pTOBAWorker->AnalyzeData ();
      *m_pLogStream << loopExec << F(": AnalyzeData(): ") << TOBA::GetResultText (result) << endl;
    }
  }

  // Analyse the received request to retrieve a worker command.
  // It is sufficient to check if a request exists, because one was only created if the worker was not busy.
  if (m_pTOBAWorker->get_ExistsRequest ())
  {
    result = m_pTOBAWorker->AnalyzeRequest ();
    *m_pLogStream << loopExec << F(": AnalyzeRequest(): ") << TOBA::GetResultText (result) << endl;
  }

  // Execute the received worker command and create a reply.
  // It usually is sufficient to check if a worker command exists, because one was only created if the worker was not busy.
  if (m_pTOBAWorker->get_ExistsWork ())
  {
    result = m_pTOBAWorker->Work ();
    *m_pLogStream << loopExec << F(": Work(): ") << TOBA::GetResultText (result) << endl;
  }

  // Send the reply that was created during analysis or work.
  // It usually is sufficient to check if a reply exists, because one was only created if the worker has finished or a failure happened.
  if (m_pTOBAWorker->get_ExistsReply ())
  {
    digitalWrite (c_PinNumber_ReceiveDisable, HIGH);
    digitalWrite (c_PinNumber_SendEnable,     HIGH);

    result = m_pTOBAWorker->SendReply ();
    *m_pLogStream << loopExec << F(": SendReply(): ") << TOBA::GetResultText (result) << endl;

    digitalWrite (c_PinNumber_SendEnable, LOW);
    digitalWrite (c_PinNumber_ReceiveDisable, LOW);
  }

  *m_pLogStream << loopExec << F(": Sleeping ") << c_Sleep_ms << " ms." << endl << endl;
  delay (c_Sleep_ms);
}
