#include <EEPROM.h>

#include "TOBAConfig.h"
#include "TOBAConfig_CustomIO.h"

//--------------------------------------------------------------------
::EResult TOBAConfig::Create (uint16_t      i_ReceiveBufferSize,
                              uint16_t      i_SendBufferSize,
                              uint16_t      i_PayloadBuffersSize,
                              const char*   i_pWorkerName,
                              uint8_t       i_WorkerNameLength,
                              uint16_t      i_EepromAddress_UCOPConfig,
                              TOBAConfig*&  o_pConfig)
{
  o_pConfig = nullptr;
  TOBAConfig* pConfig = new TOBAConfig (i_ReceiveBufferSize,
                                        i_SendBufferSize,
                                        i_PayloadBuffersSize,
                                        i_pWorkerName,
                                        i_WorkerNameLength,
                                        i_EepromAddress_UCOPConfig);

  ::EResult result = pConfig->Verify_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete pConfig;
    return result;
  }

  o_pConfig = pConfig;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig::Create (uint16_t      i_EepromAddress,
                              TOBAConfig*&  o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  uint32_t workerTypeAsNumber = 0;
  bool isOK = true;
  uint16_t address = i_EepromAddress;
  isOK &= EEPROM_GetValueAndMovePtr (address, workerTypeAsNumber);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  ::TOBAWorker::EWorkerType workerType = (::TOBAWorker::EWorkerType)workerTypeAsNumber;
  TOBAConfig* pConfig = nullptr;
  ::EResult result = CreateObject (workerType, pConfig);
  if (result != ::EResult::SUCCESS)
    return result;

  result = pConfig->ReadFromEEPROM (i_EepromAddress);
  if (result != ::EResult::SUCCESS)
  {
    delete pConfig;
    return result;
  }

  result = pConfig->Verify_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete pConfig;
    return result;
  }

  o_pConfig = pConfig;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBAConfig::TOBAConfig ()
{
}

//--------------------------------------------------------------------
TOBAConfig::~TOBAConfig ()
{
}

//--------------------------------------------------------------------
TOBAConfig::TOBAConfig (uint16_t    i_ReceiveBufferSize,
                        uint16_t    i_SendBufferSize,
                        uint16_t    i_PayloadBuffersSize,
                        const char* i_pWorkerName,
                        uint8_t     i_WorkerNameLength,
                        uint16_t    i_EepromAddress_UCOPConfig)
{
  m_ReceiveBufferSize         = i_ReceiveBufferSize;
  m_SendBufferSize            = i_SendBufferSize;
  m_PayloadBuffersSize        = i_PayloadBuffersSize;
  m_EepromAddress_UCOPConfig  = i_EepromAddress_UCOPConfig;

  if (i_pWorkerName != nullptr)
  {
    memset (m_WorkerName, 0x00, sizeof (m_WorkerName));
    memcpy (m_WorkerName, i_pWorkerName, min (sizeof (m_WorkerName), i_WorkerNameLength));
  }
}

//--------------------------------------------------------------------
::EResult TOBAConfig::CreateObject (TOBAWorker::EWorkerType i_Type,
                                    TOBAConfig*&            o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  switch (i_Type)
  {
  case TOBAWorker::EWorkerType::BuiltIn:          o_pConfig = new TOBAConfig          (); break;
  case TOBAWorker::EWorkerType::BuiltIn_CustomIO: o_pConfig = new TOBAConfig_CustomIO (); break;
  default: return (::EResult)TOBAWorker::EResult::FAIL_TOBA_WorkerTypeInvalid;
  }

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig::get_EepromConfigDataSize ()
{
  return 44;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig::get_EepromConfigChecksumSize ()
{
  return 2;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig::get_EepromAddress_UCOPConfig ()
{
  return m_EepromAddress_UCOPConfig;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig::get_PayloadBuffersSize ()
{
  return m_PayloadBuffersSize;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig::get_ReceiveBufferSize ()
{
  return m_ReceiveBufferSize;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig::get_SendBufferSize ()
{
  return m_SendBufferSize;
}

//--------------------------------------------------------------------
char* TOBAConfig::get_WorkerName ()
{
  return m_WorkerName;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig::get_WorkerNameLength ()
{
  return strnlen (m_WorkerName, sizeof (m_WorkerName));
}

//--------------------------------------------------------------------
TOBAWorker::EWorkerType TOBAConfig::get_WorkerType ()
{
  return TOBAWorker::EWorkerType::BuiltIn;
}

//--------------------------------------------------------------------
void TOBAConfig::Print ()
{
  Print_EXEC ();
}

//--------------------------------------------------------------------
::EResult TOBAConfig::WriteToEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address;
  EEPROM_SetValueAndMovePtr (address, (uint32_t)get_WorkerType ());

  ::EResult result = WriteToEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  uint16_t checksum = EEPROM_CalcChecksumCRC16 (i_Address, eepromConfigDataSize);
  EEPROM_SetValueAndMovePtr (address, checksum);

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
void TOBAConfig::Print_EXEC ()
{
  Serial << F("ReceiveBufferSize        = ") << m_ReceiveBufferSize << endl;
  Serial << F("SendBufferSize           = ") << m_SendBufferSize << endl;
  Serial << F("PayloadBuffersSize       = ") << m_PayloadBuffersSize << endl;
  Serial << F("WorkerName               = ") << _BUFPART (m_WorkerName, strnlen (m_WorkerName, sizeof (m_WorkerName))) << endl;
  Serial << F("EepromAddress_UCOPConfig = ") << m_EepromAddress_UCOPConfig << endl;
}

//--------------------------------------------------------------------
::EResult TOBAConfig::ReadFromEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_ReceiveBufferSize);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_SendBufferSize);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_PayloadBuffersSize);
  isOK &= EEPROM_GetBytesAndMovePtr (io_Address, sizeof (m_WorkerName), (uint8_t*)m_WorkerName);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig::Verify_EXEC ()
{
  if (m_ReceiveBufferSize  < c_MinRecvSendBuffersSize
  ||  m_SendBufferSize     < c_MinRecvSendBuffersSize
  ||  m_PayloadBuffersSize < c_MinPayloadRecvSendBuffersSize)
    return ::EResult::FAIL_Buffer_TooSmall;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig::WriteToEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_ReceiveBufferSize);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_SendBufferSize);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_PayloadBuffersSize);
  isOK &= EEPROM_SetBytesAndMovePtr (io_Address, sizeof (m_WorkerName), (uint8_t*)m_WorkerName);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_SetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig::ReadFromEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address + 4;
  ::EResult result = ReadFromEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  uint16_t checksumFromEEPROM = 0;
  EEPROM_GetValueAndMovePtr (address, checksumFromEEPROM);

  uint16_t checksum = EEPROM_CalcChecksumCRC16 (i_Address, eepromConfigDataSize);
  if (checksum != checksumFromEEPROM)
    return ::EResult::FAIL_Device_ConfigChecksumWrong;

  return ::EResult::SUCCESS;
}
