#include <EEPROM.h>

#include "TOBAConfig_Worker.h"
#include "TOBAConfig_CustomIO.h"

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::Create ( uint16_t            i_ReceiveBufferSize,
                                      uint16_t            i_SendBufferSize,
                                      uint16_t            i_PayloadBuffersSize,
                                      char*               i_pWorkerName,
                                      uint8_t             i_WorkerNameLength,
                                      uint16_t            i_EepromAddress_UCOPConfig,
                                      TOBAConfig_Worker*& o_pConfig)
{
  o_pConfig = nullptr;
  TOBAConfig_Worker* pConfig = new TOBAConfig_Worker (i_ReceiveBufferSize,
                                                      i_SendBufferSize,
                                                      i_PayloadBuffersSize,
                                                      i_pWorkerName,
                                                      i_WorkerNameLength,
                                                      i_EepromAddress_UCOPConfig);
  ::EResult result = pConfig->VerifyConfig_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete (pConfig);
    return result;
  }

  o_pConfig = pConfig;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::Create ( uint16_t            i_EepromAddress,
                                      TOBAConfig_Worker*& o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  uint32_t workerTypeAsNumber = 0;
  bool isOK = true;
  uint16_t address = i_EepromAddress;
  isOK &= EEPROM_GetValueAndMovePtr (address, workerTypeAsNumber);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  ::TOBA_Worker::EWorkerType workerType = (::TOBA_Worker::EWorkerType)workerTypeAsNumber;
  TOBAConfig_Worker* pConfig = nullptr;
  ::EResult result = CreateObject (workerType, pConfig);
  if (result != ::EResult::SUCCESS)
    return result;

  result = pConfig->ReadConfigFromEEPROM_exec (i_EepromAddress);
  if (result != ::EResult::SUCCESS)
    return result;

  result = pConfig->VerifyConfig_EXEC ();
  if (result != ::EResult::SUCCESS)
    return result;

  o_pConfig = pConfig;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBAConfig_Worker::TOBAConfig_Worker ()
{
}

//--------------------------------------------------------------------
TOBAConfig_Worker::~TOBAConfig_Worker ()
{
  delete m_pWorkerName;
}

//--------------------------------------------------------------------
TOBAConfig_Worker::TOBAConfig_Worker (uint16_t    i_ReceiveBufferSize,
                                      uint16_t    i_SendBufferSize,
                                      uint16_t    i_PayloadBuffersSize,
                                      char*       i_pWorkerName,
                                      uint8_t     i_WorkerNameLength,
                                      uint16_t    i_EepromAddress_UCOPConfig)
{
  m_ReceiveBufferSize         = i_ReceiveBufferSize;
  m_SendBufferSize            = i_SendBufferSize;
  m_PayloadBuffersSize        = i_PayloadBuffersSize;
  m_EepromAddress_UCOPConfig  = i_EepromAddress_UCOPConfig;

  uint8_t workerNameLength = min (c_MaxWorkerNameLength, i_WorkerNameLength);
  if (i_pWorkerName != nullptr)
  {
    m_pWorkerName = new char[workerNameLength + 1];
    memset (m_pWorkerName, 0x00, workerNameLength + 1);
    memcpy (m_pWorkerName, i_pWorkerName, workerNameLength);
  }
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::CreateObject ( TOBA_Worker::EWorkerType  i_Type,
                                            TOBAConfig_Worker*&       o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  switch (i_Type)
  {
  case TOBA_Worker::EWorkerType::BuiltIn_Basic:     o_pConfig = new TOBAConfig_Worker   (); break;
  case TOBA_Worker::EWorkerType::BuiltIn_CustomIO:  o_pConfig = new TOBAConfig_CustomIO (); break;
  default: return (::EResult)TOBA_Worker::EResult::FAIL_TOBA_WorkerTypeInvalid;
  }

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_Worker::get_EepromConfigDataSize ()
{
  return 44;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_Worker::get_EepromConfigChecksumSize ()
{
  return 2;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Worker::get_PayloadBuffersSize ()
{
  return m_PayloadBuffersSize;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Worker::get_ReceiveBufferSize ()
{
  return m_ReceiveBufferSize;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Worker::get_SendBufferSize ()
{
  return m_SendBufferSize;
}

//--------------------------------------------------------------------
char* TOBAConfig_Worker::get_WorkerName ()
{
  return m_pWorkerName;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_Worker::get_WorkerNameLength ()
{
  return strlen (m_pWorkerName);
}

//--------------------------------------------------------------------
TOBA_Worker::EWorkerType TOBAConfig_Worker::get_WorkerType ()
{
  return EWorkerType::BuiltIn_Basic;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::WriteConfigToEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address;
  EEPROM_SetValueAndMovePtr (address, (uint32_t)get_WorkerType ());

  ::EResult result = WriteConfigToEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  uint16_t checksum = EEPROM_CalcChecksumCRC16 (i_Address, eepromConfigDataSize);
  EEPROM_SetValueAndMovePtr (address, checksum);

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::ReadConfigFromEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_ReceiveBufferSize);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_SendBufferSize);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_PayloadBuffersSize);
  isOK &= EEPROM_GetBytesAndMovePtr (io_Address, c_MaxWorkerNameLength, (uint8_t*)m_pWorkerName);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::VerifyConfig_EXEC ()
{
  if (m_ReceiveBufferSize  < c_MinRecvSendBuffersSize
  ||  m_SendBufferSize     < c_MinRecvSendBuffersSize
  ||  m_PayloadBuffersSize < c_MinPayloadRecvSendBuffersSize)
    return ::EResult::FAIL_Buffer_TooSmall;

  if (m_pWorkerName == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::WriteConfigToEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_ReceiveBufferSize);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_SendBufferSize);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_PayloadBuffersSize);
  EEPROM_SetValue_FromStart (io_Address, c_MaxWorkerNameLength, 0x00);
  isOK &= EEPROM_SetBytesAndMovePtr (io_Address, get_WorkerNameLength (), m_pWorkerName  FEHLER!!);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;
  
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Worker::ReadConfigFromEEPROM_exec (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address + 4;
  ::EResult result = ReadConfigFromEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  uint16_t checksumFromEEPROM = 0;
  EEPROM_GetValueAndMovePtr (address, checksumFromEEPROM);

  uint16_t checksum = EEPROM_CalcChecksumCRC16 (i_Address, eepromConfigDataSize);
  if (checksum != checksumFromEEPROM)
    return ::EResult::FAIL_Device_ConfigChecksumWrong;

  return ::EResult::SUCCESS;
}
