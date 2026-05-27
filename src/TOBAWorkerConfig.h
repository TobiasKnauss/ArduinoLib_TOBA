#ifndef TOBAWorkerConfig_h
#define TOBAWorkerConfig_h

#include "TOBADeviceConfig.h"

//--------------------------------------------------------------------
//
// The configuration of a basic TOBA worker.
// EEPROM size: basic config size.
//--------------------------------------------------------------------
class TOBAWorkerConfig
: public TOBADeviceConfig
{
//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t            i_ReceiveBufferSize,
                            uint16_t            i_SendBufferSize,
                            uint16_t            i_PayloadBuffersSize,
                            const char*         i_pDeviceName,
                            uint8_t             i_DeviceNameLength,
                            uint16_t            i_EepromAddress_UCOPConfig,
                            TOBAWorkerConfig*&  o_pConfig);

  //-------------------- instance --------------------

  TOBAWorkerConfig ();

  virtual ~TOBAWorkerConfig ();

protected:
  //-------------------- instance --------------------

  TOBAWorkerConfig (uint16_t        i_ReceiveBufferSize,
                    uint16_t        i_SendBufferSize,
                    uint16_t        i_PayloadBuffersSize,
                    const char*     i_pDeviceName,
                    uint8_t         i_DeviceNameLength,
                    uint16_t        i_EepromAddress_UCOPConfig);

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
