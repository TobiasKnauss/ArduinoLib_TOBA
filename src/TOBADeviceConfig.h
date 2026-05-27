#ifndef TOBADeviceConfig_h
#define TOBADeviceConfig_h

#include <Result.h>
#include <MemoryTools_EEPROM.h>

#include "TOBA.h"
#include "TOBADevice.h"

//--------------------------------------------------------------------
//
// The configuration of a any TOBA device.
// EEPROM size: 44 bytes data + 2 bytes checksum.
//--------------------------------------------------------------------
class TOBADeviceConfig
{
//==================== Fields ====================
private:
  //-------------------- static --------------------

  static const uint8_t c_MinRecvSendBuffersSize        = 40;
  static const uint8_t c_MinPayloadRecvSendBuffersSize = 10;

  //-------------------- instance --------------------

  uint16_t m_SendBufferSize     = 0;
  uint16_t m_ReceiveBufferSize  = 0;
  uint16_t m_PayloadBuffersSize = 0;
  char     m_DeviceName[32];
  uint16_t m_EepromAddress_UCOPConfig = 0;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t            i_EepromAddress,
                            TOBADeviceConfig*&  o_pConfig);

  //-------------------- instance --------------------

  TOBADeviceConfig ();

  virtual ~TOBADeviceConfig ();

protected:
  //-------------------- instance --------------------

  TOBADeviceConfig (uint16_t    i_ReceiveBufferSize,
                    uint16_t    i_SendBufferSize,
                    uint16_t    i_PayloadBuffersSize,
                    const char* i_pDeviceName,
                    uint8_t     i_DeviceNameLength,
                    uint16_t    i_EepromAddress_UCOPConfig);

private:
  //-------------------- static --------------------

  static ::EResult CreateObject ( TOBADevice::EDeviceType i_Type,
                                  TOBADeviceConfig*&      o_pConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual uint8_t get_EepromConfigDataSize ();
  uint8_t         get_EepromConfigChecksumSize ();

  uint16_t get_EepromAddress_UCOPConfig ();
  uint16_t get_PayloadBuffersSize ();
  uint16_t get_ReceiveBufferSize ();
  uint16_t get_SendBufferSize ();
  char*    get_DeviceName ();
  uint8_t  get_DeviceNameLength ();

  virtual TOBADevice::EDeviceType get_DeviceType () = 0;

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  void Print ();

  ::EResult WriteToEEPROM (uint16_t i_Address);

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual void Print_EXEC ();

  virtual ::EResult ReadFromEEPROM_EXEC (uint16_t& io_Address);

  virtual ::EResult Verify_EXEC ();

  virtual ::EResult WriteToEEPROM_EXEC (uint16_t& io_Address);

//==================== Protected Methods ====================
private:
  //-------------------- instance --------------------

  ::EResult ReadFromEEPROM (uint16_t i_Address);
};

#endif
