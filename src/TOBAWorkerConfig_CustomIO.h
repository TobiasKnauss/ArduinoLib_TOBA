#ifndef TOBAWorkerConfig_CustomIO_h
#define TOBAWorkerConfig_CustomIO_h

#include "TOBAWorkerConfig.h"

//--------------------------------------------------------------------
//
// The configuration of a "Custom IO" TOBA worker.
// EEPROM size: basic config size + 1 + IO Count.
//--------------------------------------------------------------------
class TOBAWorkerConfig_CustomIO
: public TOBAWorkerConfig
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  uint8_t  m_IOCount = 0;
  uint8_t* m_pIOPins = nullptr;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t                    i_ReceiveBufferSize,
                            uint16_t                    i_SendBufferSize,
                            uint16_t                    i_PayloadBuffersSize,
                            const char*                 i_pDeviceName,
                            uint8_t                     i_DeviceNameLength,
                            uint16_t                    i_EepromAddress_UCOPConfig,
                            uint8_t                     i_IOCount,
                            const uint8_t*              i_IOPins,
                            TOBAWorkerConfig_CustomIO*& o_pConfig);

  //-------------------- instance --------------------

  TOBAWorkerConfig_CustomIO ();

  virtual ~TOBAWorkerConfig_CustomIO ();

protected:
  //-------------------- instance --------------------

  TOBAWorkerConfig_CustomIO ( uint16_t        i_ReceiveBufferSize,
                              uint16_t        i_SendBufferSize,
                              uint16_t        i_PayloadBuffersSize,
                              const char*     i_pDeviceName,
                              uint8_t         i_DeviceNameLength,
                              uint16_t        i_EepromAddress_UCOPConfig,
                              uint8_t         i_IOCount,
                              const uint8_t*  i_IOPins);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual uint8_t get_EepromConfigDataSize () override;

  uint8_t  get_IOCount ();
  uint8_t  get_IOPin (uint8_t i_IONumber);
  uint8_t* get_IOPins ();

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
