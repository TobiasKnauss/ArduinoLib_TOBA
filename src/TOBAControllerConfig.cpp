#include "TOBAControllerConfig.h"

//--------------------------------------------------------------------
::EResult TOBAControllerConfig::Create (uint16_t                i_ReceiveBufferSize,
                                        uint16_t                i_SendBufferSize,
                                        uint16_t                i_PayloadBuffersSize,
                                        const char*             i_pDeviceName,
                                        uint8_t                 i_DeviceNameLength,
                                        uint16_t                i_EepromAddress_UCOPConfig,
                                        TOBAControllerConfig*&  o_pConfig)
{
  o_pConfig = nullptr;
  TOBAControllerConfig* pConfig = new TOBAControllerConfig (i_ReceiveBufferSize,
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
TOBAControllerConfig::TOBAControllerConfig ()
{
}

//--------------------------------------------------------------------
TOBAControllerConfig::~TOBAControllerConfig ()
{
}

//--------------------------------------------------------------------
TOBAControllerConfig::TOBAControllerConfig (uint16_t    i_ReceiveBufferSize,
                                            uint16_t    i_SendBufferSize,
                                            uint16_t    i_PayloadBuffersSize,
                                            const char* i_pDeviceName,
                                            uint8_t     i_DeviceNameLength,
                                            uint16_t    i_EepromAddress_UCOPConfig)
: TOBADeviceConfig (i_ReceiveBufferSize,
                    i_SendBufferSize,
                    i_PayloadBuffersSize,
                    i_pDeviceName,
                    i_DeviceNameLength,
                    i_EepromAddress_UCOPConfig)
{
}

//--------------------------------------------------------------------
uint8_t TOBAControllerConfig::get_EepromConfigDataSize ()
{
  return TOBADeviceConfig::get_EepromConfigDataSize ();
}

//--------------------------------------------------------------------
TOBADevice::EDeviceType TOBAControllerConfig::get_DeviceType ()
{
  return TOBADevice::EDeviceType::Controller;
}

//--------------------------------------------------------------------
void TOBAControllerConfig::Print_EXEC ()
{
  TOBADeviceConfig::Print_EXEC ();
}

//--------------------------------------------------------------------
::EResult TOBAControllerConfig::ReadFromEEPROM_EXEC (uint16_t& io_Address)
{
  return TOBADeviceConfig::ReadFromEEPROM_EXEC (io_Address);
}

//--------------------------------------------------------------------
::EResult TOBAControllerConfig::Verify_EXEC ()
{
  return TOBADeviceConfig::Verify_EXEC ();
}

//--------------------------------------------------------------------
::EResult TOBAControllerConfig::WriteToEEPROM_EXEC (uint16_t& io_Address)
{
  return TOBADeviceConfig::WriteToEEPROM_EXEC (io_Address);
}
