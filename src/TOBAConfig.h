#ifndef TOBAConfig_h
#define TOBAConfig_h

#include <Result.h>
#include <MemoryTools_EEPROM.h>

#include "TOBAWorker.h"

//--------------------------------------------------------------------
//
// The configuration of a basic TOBA worker.
// EEPROM size: 44 bytes data + 2 bytes checksum.
//--------------------------------------------------------------------
class TOBAConfig
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
  char     m_WorkerName[32];
  uint16_t m_EepromAddress_UCOPConfig = 0;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t      i_ReceiveBufferSize,
                            uint16_t      i_SendBufferSize,
                            uint16_t      i_PayloadBuffersSize,
                            const char*   i_pWorkerName,
                            uint8_t       i_WorkerNameLength,
                            uint16_t      i_EepromAddress_UCOPConfig,
                            TOBAConfig*&  o_pConfig);

  static ::EResult Create ( uint16_t            i_EepromAddress,
                            TOBAConfig*&  o_pConfig);

  //-------------------- instance --------------------

  TOBAConfig ();

  virtual ~TOBAConfig ();

protected:
  //-------------------- instance --------------------

  TOBAConfig (uint16_t    i_ReceiveBufferSize,
              uint16_t    i_SendBufferSize,
              uint16_t    i_PayloadBuffersSize,
              const char* i_pWorkerName,
              uint8_t     i_WorkerNameLength,
              uint16_t    i_EepromAddress_UCOPConfig);

private:
  //-------------------- static --------------------

  static ::EResult CreateObject ( TOBAWorker::EWorkerType i_Type,
                                  TOBAConfig*&            o_pConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual uint8_t get_EepromConfigDataSize ();
  uint8_t         get_EepromConfigChecksumSize ();

  uint16_t get_EepromAddress_UCOPConfig ();
  uint16_t get_PayloadBuffersSize ();
  uint16_t get_ReceiveBufferSize ();
  uint16_t get_SendBufferSize ();
  char*    get_WorkerName ();
  uint8_t  get_WorkerNameLength ();

  virtual TOBAWorker::EWorkerType get_WorkerType ();

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
