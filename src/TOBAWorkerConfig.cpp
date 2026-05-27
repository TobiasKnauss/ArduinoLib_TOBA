#include "TOBAWorkerConfig.h"

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig::Create (uint16_t            i_ReceiveBufferSize,
                                    uint16_t            i_SendBufferSize,
                                    uint16_t            i_PayloadBuffersSize,
                                    const char*         i_pDeviceName,
                                    uint8_t             i_DeviceNameLength,
                                    uint16_t            i_EepromAddress_UCOPConfig,
                                    TOBAWorkerConfig*&  o_pConfig)
{
  o_pConfig = nullptr;
  TOBAWorkerConfig* pConfig = new TOBAWorkerConfig (i_ReceiveBufferSize,
                                                    i_SendBufferSize,
                                                    i_PayloadBuffersSize,
                                                    i_pDeviceName,
                                                    i_DeviceNameLength,
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
TOBAWorkerConfig::TOBAWorkerConfig ()
{
}

//--------------------------------------------------------------------
TOBAWorkerConfig::~TOBAWorkerConfig ()
{
}

//--------------------------------------------------------------------
TOBAWorkerConfig::TOBAWorkerConfig (uint16_t        i_ReceiveBufferSize,
                                    uint16_t        i_SendBufferSize,
                                    uint16_t        i_PayloadBuffersSize,
                                    const char*     i_pDeviceName,
                                    uint8_t         i_DeviceNameLength,
                                    uint16_t        i_EepromAddress_UCOPConfig)
: TOBADeviceConfig (i_ReceiveBufferSize,
                    i_SendBufferSize,
                    i_PayloadBuffersSize,
                    i_pDeviceName,
                    i_DeviceNameLength,
                    i_EepromAddress_UCOPConfig)
{
}

//--------------------------------------------------------------------
uint8_t TOBAWorkerConfig::get_EepromConfigDataSize ()
{
  return TOBADeviceConfig::get_EepromConfigDataSize ();
}

//--------------------------------------------------------------------
TOBADevice::EDeviceType TOBAWorkerConfig::get_DeviceType ()
{
  return TOBADevice::EDeviceType::Worker;
}

//--------------------------------------------------------------------
void TOBAWorkerConfig::Print_EXEC ()
{
  TOBADeviceConfig::Print_EXEC ();
}

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig::ReadFromEEPROM_EXEC (uint16_t& io_Address)
{
  return TOBADeviceConfig::ReadFromEEPROM_EXEC (io_Address);
}

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig::Verify_EXEC ()
{
  return TOBADeviceConfig::Verify_EXEC ();
}

//--------------------------------------------------------------------
::EResult TOBAWorkerConfig::WriteToEEPROM_EXEC (uint16_t& io_Address)
{
  return TOBADeviceConfig::WriteToEEPROM_EXEC (io_Address);
}
