#include <Streaming.h>

#include <TOBAConfig.h>

const bool     c_WriteConfig = false;  // <--- Set this flag to TRUE if the config has to be changed.
const uint16_t c_EepromOffset = 00;

const uint16_t m_ReceiveBufferSize        = 80;
const uint16_t m_SendBufferSize           = 50;
const uint16_t m_PayloadBuffersSize       = 20;
const char     m_WorkerName[]             = "Basic Worker #1";
const uint8_t  m_WorkerNameLength         = sizeof (m_WorkerName);
const uint16_t m_EepromAddress_UCOPConfig = 60;

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  TOBAConfig* pTOBAConfig1 = nullptr;
  EResult result = TOBAConfig::Create ( m_ReceiveBufferSize,
                                        m_SendBufferSize,
                                        m_PayloadBuffersSize,
                                        m_WorkerName,
                                        m_WorkerNameLength,
                                        m_EepromAddress_UCOPConfig,
                                        pTOBAConfig1);
  Serial << "TOBAConfig.Create(..data..), Result: " << (int)result << " = " << TOBAWorker::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  pTOBAConfig1->Print ();

  result = pTOBAConfig1->WriteToEEPROM (c_EepromOffset);
  Serial << "TOBAConfig.WriteToEEPROM, Result: " << (int)result << " = " << TOBAWorker::GetResultText (result) << endl;
  if (result != EResult::SUCCESS)
    return;

  TOBAConfig* pTOBAConfig2 = nullptr;
  result = TOBAConfig::Create (c_EepromOffset, pTOBAConfig2);
  Serial << "TOBAConfig.Create(..eepromAddress..), Result: " << (int)result << " = " << TOBAWorker::GetResultText (result) << endl;
  if (result == EResult::SUCCESS)
    pTOBAConfig2->Print ();
}

void loop ()
{
}
