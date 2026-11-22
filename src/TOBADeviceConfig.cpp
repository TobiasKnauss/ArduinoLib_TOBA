#include <EEPROM.h>

#include "TOBADeviceConfig.h"
#include "TOBAControllerConfig.h"
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
  isOK &= EepromTools::ReadValueAndMovePtr (address, deviceTypeAsNumber, c_InvertByteOrder);
  if (!isOK)
    return ::EResult::FAIL_Eeprom_ReadValue;

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
  case TOBADevice::EDeviceType::Controller:      o_pConfig = new TOBAControllerConfig      (); break;
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
bool TOBADeviceConfig::Print (Stream* i_pOutput)
{
  if (i_pOutput == nullptr)
    return false;

  Print_EXEC (i_pOutput);
  return true;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::WriteToEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_Eeprom_IndexOutsideRange;

  uint16_t address = i_Address;
  EepromTools::WriteValueAndMovePtr (address, (uint32_t)get_DeviceType (), c_InvertByteOrder);

  ::EResult result = WriteToEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  if (eepromConfigDataSize != address - i_Address)
    return ::EResult::FAIL_Device_ConfigInvalid;
  uint16_t checksum = 0;
  if (!EepromTools::CalcChecksumCRC16_From (i_Address, eepromConfigDataSize, checksum))
    return ::EResult::FAIL_Eeprom_CalcChecksum;

  EepromTools::WriteValueAndMovePtr (address, checksum, c_InvertByteOrder);

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
void TOBADeviceConfig::Print_EXEC (Stream* i_pOutput)
{
  *i_pOutput << F("ReceiveBufferSize        = ") << m_ReceiveBufferSize << endl;
  *i_pOutput << F("SendBufferSize           = ") << m_SendBufferSize << endl;
  *i_pOutput << F("PayloadBuffersSize       = ") << m_PayloadBuffersSize << endl;
  *i_pOutput << F("DeviceName               = ") << _BYTES (m_DeviceName, strnlen (m_DeviceName, sizeof (m_DeviceName))) << endl;
  *i_pOutput << F("EepromAddress_UCOPConfig = ") << m_EepromAddress_UCOPConfig << endl;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::ReadFromEEPROM_EXEC (uint16_t& io_Address)
{
  bool isOK = true;
  isOK &= EepromTools::ReadValueAndMovePtr (io_Address, m_ReceiveBufferSize,  c_InvertByteOrder);
  isOK &= EepromTools::ReadValueAndMovePtr (io_Address, m_SendBufferSize,     c_InvertByteOrder);
  isOK &= EepromTools::ReadValueAndMovePtr (io_Address, m_PayloadBuffersSize, c_InvertByteOrder);
  isOK &= EepromTools::ReadBytesAndMovePtr (io_Address, sizeof (m_DeviceName), (uint8_t*)m_DeviceName, false);
  isOK &= EepromTools::ReadValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig, c_InvertByteOrder);
  if (!isOK)
    return ::EResult::FAIL_Eeprom_ReadValue;

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
  isOK &= EepromTools::WriteValueAndMovePtr (io_Address, m_ReceiveBufferSize,  c_InvertByteOrder);
  isOK &= EepromTools::WriteValueAndMovePtr (io_Address, m_SendBufferSize,     c_InvertByteOrder);
  isOK &= EepromTools::WriteValueAndMovePtr (io_Address, m_PayloadBuffersSize, c_InvertByteOrder);
  isOK &= EepromTools::WriteBytesAndMovePtr (io_Address, sizeof (m_DeviceName), (uint8_t*)m_DeviceName, false);
  isOK &= EepromTools::WriteValueAndMovePtr (io_Address, m_EepromAddress_UCOPConfig, c_InvertByteOrder);
  if (!isOK)
    return ::EResult::FAIL_Eeprom_WriteValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBADeviceConfig::ReadFromEEPROM (uint16_t i_Address)
{
  uint8_t eepromConfigDataSize = get_EepromConfigDataSize ();
  uint8_t eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_Eeprom_IndexOutsideRange;

  uint16_t address = i_Address + sizeof (TOBADevice::EDeviceType);
  ::EResult result = ReadFromEEPROM_EXEC (address);
  if (result != ::EResult::SUCCESS)
    return result;

  // re-check the config data size based on the just read data.
  eepromConfigDataSize = get_EepromConfigDataSize (); // re-calc for correct EEPROM checksum calcualtion.
  eepromConfigTotalSize = eepromConfigDataSize + get_EepromConfigChecksumSize ();
  if (eepromConfigTotalSize + i_Address > EEPROM.length ())
    return ::EResult::FAIL_Eeprom_IndexOutsideRange;

  if (eepromConfigDataSize != address - i_Address)
    return ::EResult::FAIL_Device_ConfigInvalid;

  uint16_t checksumFromEEPROM;
  uint16_t checksum = 0;
  if (!EepromTools::ReadValueAndMovePtr (address, checksumFromEEPROM, c_InvertByteOrder))
    return ::EResult::FAIL_Eeprom_ReadValue;
  if (!EepromTools::CalcChecksumCRC16_From (i_Address, eepromConfigDataSize, checksum))
    return ::EResult::FAIL_Eeprom_CalcChecksum;
  if (checksum != checksumFromEEPROM)
    return ::EResult::FAIL_Device_ConfigChecksumWrong;

  return ::EResult::SUCCESS;
}
