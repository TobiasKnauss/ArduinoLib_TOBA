#include "TOBAWorkerConfig_CustomIO.h"

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig_CustomIO::Create ( uint16_t                    i_ReceiveBufferSize,
                                              uint16_t                    i_SendBufferSize,
                                              uint16_t                    i_PayloadBuffersSize,
                                              const char*                 i_pDeviceName,
                                              uint8_t                     i_DeviceNameLength,
                                              uint16_t                    i_EepromAddress_UCOPConfig,
                                              uint8_t                     i_IOCount,
                                              const uint8_t*              i_pIOPins,
                                              TOBAWorkerConfig_CustomIO*& o_pConfig)
{
  o_pConfig = nullptr;
  TOBAWorkerConfig_CustomIO* pConfig = new TOBAWorkerConfig_CustomIO (i_ReceiveBufferSize,
                                                                      i_SendBufferSize,
                                                                      i_PayloadBuffersSize,
                                                                      i_pDeviceName,
                                                                      i_DeviceNameLength,
                                                                      i_EepromAddress_UCOPConfig,
                                                                      i_IOCount,
                                                                      i_pIOPins);

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
TOBAWorkerConfig_CustomIO::TOBAWorkerConfig_CustomIO ()
{
}

//--------------------------------------------------------------------
TOBAWorkerConfig_CustomIO::~TOBAWorkerConfig_CustomIO ()
{
  DeleteObject (m_pIOPins);
}

//--------------------------------------------------------------------
TOBAWorkerConfig_CustomIO::TOBAWorkerConfig_CustomIO (uint16_t        i_ReceiveBufferSize,
                                                      uint16_t        i_SendBufferSize,
                                                      uint16_t        i_PayloadBuffersSize,
                                                      const char*     i_pDeviceName,
                                                      uint8_t         i_DeviceNameLength,
                                                      uint16_t        i_EepromAddress_UCOPConfig,
                                                      uint8_t         i_IOCount,
                                                      const uint8_t*  i_pIOPins)
: TOBAWorkerConfig (i_ReceiveBufferSize,
                    i_SendBufferSize,
                    i_PayloadBuffersSize,
                    i_pDeviceName,
                    i_DeviceNameLength,
                    i_EepromAddress_UCOPConfig)
{
  m_IOCount = i_IOCount;

  if (i_pIOPins != nullptr)
  {
    m_pIOPins  = new uint8_t[i_IOCount];
    memcpy (m_pIOPins, i_pIOPins, i_IOCount);
  }
}

//--------------------------------------------------------------------
uint8_t TOBAWorkerConfig_CustomIO::get_EepromConfigDataSize ()
{
  return TOBAWorkerConfig::get_EepromConfigDataSize ()
       + 1
       + m_IOCount;
}

//--------------------------------------------------------------------
uint8_t TOBAWorkerConfig_CustomIO::get_IOCount ()
{
  return m_IOCount;
}

//--------------------------------------------------------------------
uint8_t TOBAWorkerConfig_CustomIO::get_IOPin (uint8_t i_IONumber)
{
  if (i_IONumber < 1
  ||  i_IONumber > m_IOCount)
    return 0;

  return m_pIOPins[i_IONumber];
}

//--------------------------------------------------------------------
uint8_t* TOBAWorkerConfig_CustomIO::get_IOPins ()
{
  return m_pIOPins;
}

//--------------------------------------------------------------------
TOBADevice::EDeviceType TOBAWorkerConfig_CustomIO::get_DeviceType ()
{
  return TOBADevice::EDeviceType::Worker_CustomIO;
}

//--------------------------------------------------------------------
void TOBAWorkerConfig_CustomIO::Print_EXEC ()
{
  TOBAWorkerConfig::Print_EXEC ();

  Serial << F("IOCount = ") << m_IOCount << endl;
  for (int index = 0; index < m_IOCount - 1; index++)
    Serial << F("IOPins = ") << m_pIOPins[index] << ", ";
  if (m_IOCount == 0)
    Serial << F("---") << endl;
  else
    Serial << m_pIOPins[m_IOCount - 1] << endl;
}

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig_CustomIO::ReadFromEEPROM_EXEC (uint16_t& io_Address)
{
  ::EResult result = TOBAWorkerConfig::ReadFromEEPROM_EXEC (io_Address);
  if (result != ::EResult::SUCCESS)
    return result;

  bool isOK = true;
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_IOCount);
  delete (m_pIOPins);
  m_pIOPins = new uint8_t[m_IOCount];
  isOK &= EEPROM_GetBytesAndMovePtr (io_Address, m_IOCount, m_pIOPins);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig_CustomIO::Verify_EXEC ()
{
  ::EResult result = TOBAWorkerConfig::Verify_EXEC ();
  if (result != ::EResult::SUCCESS)
    return result;

  if (m_pIOPins == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig_CustomIO::WriteToEEPROM_EXEC (uint16_t& io_Address)
{
  ::EResult result = TOBAWorkerConfig::WriteToEEPROM_EXEC (io_Address);
  if (result != ::EResult::SUCCESS)
    return result;

  bool isOK = true;
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_IOCount);
  isOK &= EEPROM_SetBytesAndMovePtr (io_Address, m_IOCount, m_pIOPins);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}
