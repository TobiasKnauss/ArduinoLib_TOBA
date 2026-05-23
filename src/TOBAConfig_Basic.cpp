#include <EEPROM.h>

#include "TOBAConfig_Basic.h"
#include "TOBAConfig_CustomIO.h"

//--------------------------------------------------------------------
::EResult TOBAConfig_Basic::Create (uint16_t            i_ReceiveBufferSize,
                                    uint16_t            i_SendBufferSize,
                                    uint16_t            i_PayloadBuffersSize,
                                    char*               i_pWorkerName,
                                    uint8_t             i_WorkerNameLength,
                                    uint16_t            i_EepromAddress_UCOPConfig,
                                    TOBAConfig_Basic*&  o_pConfig)
{
  o_pConfig = nullptr;
  TOBAConfig_Basic* pConfig = new TOBAConfig_Basic (i_ReceiveBufferSize,
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
::EResult TOBAConfig_Basic::Create ( uint16_t            i_EepromAddress,
                                      TOBAConfig_Basic*& o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  uint32_t workerTypeAsNumber = 0;
  bool isOK = true;
  uint16_t address = i_EepromAddress;
  isOK &= EEPROM_GetValueAndMovePtr (address, workerTypeAsNumber);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  ::TOBAWorker_Basic::EWorkerType workerType = (::TOBAWorker_Basic::EWorkerType)workerTypeAsNumber;
  TOBAConfig_Basic* pConfig = nullptr;
  ::EResult result = CreateObject (workerType, pConfig);
  if (result != ::EResult::SUCCESS)
    return result;

  result = pConfig->ReadFromEEPROM_exec (address);
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
TOBAConfig_Basic::TOBAConfig_Basic ()
{
}

//--------------------------------------------------------------------
TOBAConfig_Basic::~TOBAConfig_Basic ()
{
}

//--------------------------------------------------------------------
TOBAConfig_Basic::TOBAConfig_Basic (uint16_t  i_ReceiveBufferSize,
                                    uint16_t  i_SendBufferSize,
                                    uint16_t  i_PayloadBuffersSize,
                                    char*     i_pWorkerName,
                                    uint8_t   i_WorkerNameLength,
                                    uint16_t  i_EepromAddress_UCOPConfig)
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
::EResult TOBAConfig_Basic::CreateObject (TOBAWorker_Basic::EWorkerType i_Type,
                                          TOBAConfig_Basic*&            o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  switch (i_Type)
  {
  case TOBAWorker_Basic::EWorkerType::BuiltIn_Basic:     o_pConfig = new TOBAConfig_Basic   (); break;
  case TOBAWorker_Basic::EWorkerType::BuiltIn_CustomIO:  o_pConfig = new TOBAConfig_CustomIO (); break;
  default: return (::EResult)TOBAWorker_Basic::EResult::FAIL_TOBA_WorkerTypeInvalid;
  }

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_Basic::get_EepromConfigDataSize ()
{
  return 44;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_Basic::get_EepromConfigChecksumSize ()
{
  return 2;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Basic::get_EepromAddress_UCOPConfig ()
{
  return m_EepromAddress_UCOPConfig;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Basic::get_PayloadBuffersSize ()
{
  return m_PayloadBuffersSize;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Basic::get_ReceiveBufferSize ()
{
  return m_ReceiveBufferSize;
}

//--------------------------------------------------------------------
uint16_t TOBAConfig_Basic::get_SendBufferSize ()
{
  return m_SendBufferSize;
}

//--------------------------------------------------------------------
char* TOBAConfig_Basic::get_WorkerName ()
{
  return m_WorkerName;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_Basic::get_WorkerNameLength ()
{
  return sizeof (m_WorkerName);
}

//--------------------------------------------------------------------
TOBAWorker_Basic::EWorkerType TOBAConfig_Basic::get_WorkerType ()
{
  return TOBAWorker_Basic::EWorkerType::BuiltIn_Basic;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Basic::WriteToEEPROM (uint16_t i_Address)
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
::EResult TOBAConfig_Basic::ReadFromEEPROM_EXEC (uint16_t& io_Address)
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
::EResult TOBAConfig_Basic::Verify_EXEC ()
{
  if (m_ReceiveBufferSize  < c_MinRecvSendBuffersSize
  ||  m_SendBufferSize     < c_MinRecvSendBuffersSize
  ||  m_PayloadBuffersSize < c_MinPayloadRecvSendBuffersSize)
    return ::EResult::FAIL_Buffer_TooSmall;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_Basic::WriteToEEPROM_EXEC (uint16_t& io_Address)
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
::EResult TOBAConfig_Basic::ReadFromEEPROM_exec (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address;
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
