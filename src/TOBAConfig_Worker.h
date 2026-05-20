#ifndef TOBAConfig_Worker_h
#define TOBAConfig_Worker_h

#include <Result.h>
#include <MemoryTools_EEPROM.h>

#include "TOBA_Worker.h"

//--------------------------------------------------------------------
class TOBAConfig_Worker
{
//==================== Fields ====================
private:
  //-------------------- static --------------------

  static const uint8_t c_MinRecvSendBuffersSize        = 40;
  static const uint8_t c_MinPayloadRecvSendBuffersSize = 10;

  static const uint8_t c_MaxWorkerNameLength = 32;

  //-------------------- instance --------------------

  uint16_t m_SendBufferSize     = 0;
  uint16_t m_ReceiveBufferSize  = 0;
  uint16_t m_PayloadBuffersSize = 0;
  char*    m_pWorkerName        = nullptr;
  uint16_t m_EepromAddress_UCOPConfig = 0;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t            i_ReceiveBufferSize,
                            uint16_t            i_SendBufferSize,
                            uint16_t            i_PayloadBuffersSize,
                            char*               i_pWorkerName,
                            uint8_t             i_WorkerNameLength,
                            uint16_t            i_EepromAddress_UCOPConfig,
                            TOBAConfig_Worker*& o_pConfig);

  static ::EResult Create ( uint16_t            i_EepromAddress,
                            TOBAConfig_Worker*& o_pConfig);

  //-------------------- instance --------------------

  TOBAConfig_Worker ();

  virtual ~TOBAConfig_Worker ();

protected:
  //-------------------- instance --------------------

  TOBAConfig_Worker ( uint16_t    i_ReceiveBufferSize,
                      uint16_t    i_SendBufferSize,
                      uint16_t    i_PayloadBuffersSize,
                      char*       i_pWorkerName,
                      uint8_t     i_WorkerNameLength,
                      uint16_t    i_EepromAddress_UCOPConfig);

private:
  //-------------------- static --------------------

  static ::EResult CreateObject ( TOBA_Worker::EWorkerType  i_Type,
                                  TOBAConfig_Worker*&       o_pConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual uint8_t get_EepromConfigDataSize ();
  uint8_t         get_EepromConfigChecksumSize ();

  uint16_t get_PayloadBuffersSize ();
  uint16_t get_ReceiveBufferSize ();
  uint16_t get_SendBufferSize ();
  char*    get_WorkerName ();
  uint8_t  get_WorkerNameLength ();

  virtual TOBA_Worker::EWorkerType get_WorkerType ();

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  ::EResult WriteConfigToEEPROM (uint16_t i_Address);

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult ReadConfigFromEEPROM_EXEC (uint16_t& io_Address);

  virtual ::EResult VerifyConfig_EXEC ();

  virtual ::EResult WriteConfigToEEPROM_EXEC (uint16_t& io_Address);

//==================== Protected Methods ====================
private:
  //-------------------- instance --------------------

  ::EResult ReadConfigFromEEPROM_exec (uint16_t i_Address);
};

#endif
