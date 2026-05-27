#include <EEPROM.h>

#include "TOBADeviceConfig.h"
#include "TOBAWorkerConfig.h"
#include "TOBAWorkerConfig_CustomIO.h"

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::Create (uint16_t            i_EepromAddress,
                                    TOBADeviceConfig*&  o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  uint32_t deviceTypeAsNumber;
  bool isOK = true;
  uint16_t address = i_EepromAddress;
  isOK &= EEPROM_GetValueAndMovePtr (address, deviceTypeAsNumber);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  ::TOBADevice::EDeviceType deviceType = (::TOBADevice::EDeviceType)deviceTypeAsNumber;
  TOBADeviceConfig* pConfig = nullptr;
  ::EResult result = CreateObject (deviceType, pConfig);
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
TOBADeviceConfig::TOBADeviceConfig ()
{
}

//--------------------------------------------------------------------
TOBADeviceConfig::~TOBADeviceConfig ()
{
}

//--------------------------------------------------------------------
TOBADeviceConfig::TOBADeviceConfig (uint16_t    i_ReceiveBufferSize,
                                    uint16_t    i_SendBufferSize,
                                    uint16_t    i_PayloadBuffersSize,
                                    const char* i_pDeviceName,
                                    uint8_t     i_DeviceNameLength,
                                    uint16_t    i_EepromAddress_UCOPConfig)
{
  m_ReceiveBufferSize         = i_ReceiveBufferSize;
  m_SendBufferSize            = i_SendBufferSize;
  m_PayloadBuffersSize        = i_PayloadBuffersSize;
  m_EepromAddress_UCOPConfig  = i_EepromAddress_UCOPConfig;

  if (i_pDeviceName != nullptr)
  {
    memset (m_DeviceName, 0x00, sizeof (m_DeviceName));
    memcpy (m_DeviceName, i_pDeviceName, min (sizeof (m_DeviceName), i_DeviceNameLength));
  }
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::CreateObject (TOBADevice::EDeviceType i_Type,
                                          TOBADeviceConfig*&      o_pConfig)
{
  if (o_pConfig != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  switch (i_Type)
  {
  case TOBADevice::EDeviceType::Worker:          o_pConfig = new TOBAWorkerConfig          (); break;
  case TOBADevice::EDeviceType::Worker_CustomIO: o_pConfig = new TOBAWorkerConfig_CustomIO (); break;
  default: return (::EResult)TOBA::EResult::FAIL_TOBA_DeviceTypeInvalid;
  }

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
uint8_t TOBADeviceConfig::get_EepromConfigDataSize ()
{
  return 44;
}

//--------------------------------------------------------------------
uint8_t TOBADeviceConfig::get_EepromConfigChecksumSize ()
{
  return 2;
}

//--------------------------------------------------------------------
uint16_t TOBADeviceConfig::get_EepromAddress_UCOPConfig ()
{
  return m_EepromAddress_UCOPConfig;
}

//--------------------------------------------------------------------
uint16_t TOBADeviceConfig::get_PayloadBuffersSize ()
{
  return m_PayloadBuffersSize;
}

//--------------------------------------------------------------------
uint16_t TOBADeviceConfig::get_ReceiveBufferSize ()
{
  return m_ReceiveBufferSize;
}

//--------------------------------------------------------------------
uint16_t TOBADeviceConfig::get_SendBufferSize ()
{
  return m_SendBufferSize;
}

//--------------------------------------------------------------------
char* TOBADeviceConfig::get_DeviceName ()
{
  return m_DeviceName;
}

//--------------------------------------------------------------------
uint8_t TOBADeviceConfig::get_DeviceNameLength ()
{
  return strnlen (m_DeviceName, sizeof (m_DeviceName));
}

//--------------------------------------------------------------------
void TOBADeviceConfig::Print ()
{
  Print_EXEC ();
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::WriteToEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address;
  EEPROM_SetValueAndMovePtr (address, (uint32_t)get_DeviceType ());

  ::EResult result = WriteToEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  uint16_t checksum = EEPROM_CalcChecksumCRC16 (i_Address, eepromConfigDataSize);
  EEPROM_SetValueAndMovePtr (address, checksum);

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
void TOBADeviceConfig::Print_EXEC ()
{
  Serial << F("ReceiveBufferSize        = ") << m_ReceiveBufferSize << endl;
  Serial << F("SendBufferSize           = ") << m_SendBufferSize << endl;
  Serial << F("PayloadBuffersSize       = ") << m_PayloadBuffersSize << endl;
  Serial << F("DeviceName               = ") << _BUFPART (m_DeviceName, strnlen (m_DeviceName, sizeof (m_DeviceName))) << endl;
  Serial << F("EepromAddress_UCOPConfig = ") << m_EepromAddress_UCOPConfig << endl;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::ReadFromEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_ReceiveBufferSize);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_SendBufferSize);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_PayloadBuffersSize);
  isOK &= EEPROM_GetBytesAndMovePtr (io_Address, sizeof (m_DeviceName), (uint8_t*)m_DeviceName);
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::Verify_EXEC ()
{
  if (m_ReceiveBufferSize  < c_MinRecvSendBuffersSize
  ||  m_SendBufferSize     < c_MinRecvSendBuffersSize
  ||  m_PayloadBuffersSize < c_MinPayloadRecvSendBuffersSize)
    return ::EResult::FAIL_Buffer_TooSmall;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::WriteToEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_ReceiveBufferSize);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_SendBufferSize);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_PayloadBuffersSize);
  isOK &= EEPROM_SetBytesAndMovePtr (io_Address, sizeof (m_DeviceName), (uint8_t*)m_DeviceName);
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_SetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::ReadFromEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_EEPROM_IndexOutsideRange;

  uint16_t address = i_Address + 4;
  ::EResult result = ReadFromEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  uint16_t checksumFromEEPROM;
  EEPROM_GetValueAndMovePtr (address, checksumFromEEPROM);

  uint16_t checksum = EEPROM_CalcChecksumCRC16 (i_Address, eepromConfigDataSize);
  if (checksum != checksumFromEEPROM)
    return ::EResult::FAIL_Device_ConfigChecksumWrong;

  return ::EResult::SUCCESS;
}
