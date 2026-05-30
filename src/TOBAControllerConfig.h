#ifndef TOBAControllerConfig_h
#define TOBAControllerConfig_h

#include "TOBADeviceConfig.h"

//--------------------------------------------------------------------
//
// The configuration of a basic TOBA controller.
// EEPROM size: basic config size.
//--------------------------------------------------------------------
class TOBAControllerConfig
: public TOBADeviceConfig
{
//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t                i_ReceiveBufferSize,
                            uint16_t                i_SendBufferSize,
                            uint16_t                i_PayloadBuffersSize,
                            const char*             i_pDeviceName,
                            uint8_t                 i_DeviceNameLength,
                            uint16_t                i_EepromAddress_UCOPConfig,
                            TOBAControllerConfig*&  o_pConfig);

  //-------------------- instance --------------------

  TOBAControllerConfig ();

  virtual ~TOBAControllerConfig ();

protected:
  //-------------------- instance --------------------

  TOBAControllerConfig (uint16_t    i_ReceiveBufferSize,
                        uint16_t    i_SendBufferSize,
                        uint16_t    i_PayloadBuffersSize,
                        const char* i_pDeviceName,
                        uint8_t     i_DeviceNameLength,
                        uint16_t    i_EepromAddress_UCOPConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual uint8_t get_EepromConfigDataSize () override;

  virtual TOBADevice::EDeviceType get_DeviceType () override;

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual void Print_EXEC () override;

  virtual ::EResult ReadFromEEPROM_EXEC (uint16_t& io_Address) override;

  virtual ::EResult Verify_EXEC () override;

  virtual ::EResult WriteToEEPROM_EXEC (uint16_t& io_Address) override;
};

#endif
