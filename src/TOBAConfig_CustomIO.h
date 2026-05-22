#ifndef TOBAConfig_CustomIO_h
#define TOBAConfig_CustomIO_h

#include "TOBAConfig_Basic.h"

//--------------------------------------------------------------------
class TOBAConfig_CustomIO
: public TOBAConfig_Basic
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  uint8_t  m_IOCount = 0;
  uint8_t* m_pIOPins = nullptr;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( uint16_t              i_ReceiveBufferSize,
                            uint16_t              i_SendBufferSize,
                            uint16_t              i_PayloadBuffersSize,
                            char*                 i_pWorkerName,
                            uint8_t               i_WorkerNameLength,
                            uint16_t              i_EepromAddress_UCOPConfig,
                            uint8_t               i_IOCount,
                            uint8_t*              i_IOPins,
                            TOBAConfig_CustomIO*& o_pConfig);

  //-------------------- instance --------------------

  TOBAConfig_CustomIO ();

  virtual ~TOBAConfig_CustomIO ();

protected:
  //-------------------- instance --------------------

  TOBAConfig_CustomIO ( uint16_t i_ReceiveBufferSize,
                        uint16_t i_SendBufferSize,
                        uint16_t i_PayloadBuffersSize,
                        char*    i_pWorkerName,
                        uint8_t  i_WorkerNameLength,
                        uint16_t i_EepromAddress_UCOPConfig,
                        uint8_t  i_IOCount,
                        uint8_t* i_IOPins);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual uint8_t get_EepromConfigDataSize () override;

  uint8_t  get_IOCount ();
  uint8_t* get_IOPins ();

  virtual TOBAWorker_Basic::EWorkerType get_WorkerType () override;

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult ReadFromEEPROM_EXEC (uint16_t& io_Address) override;

  virtual ::EResult Verify_EXEC () override;

  virtual ::EResult WriteToEEPROM_EXEC (uint16_t& io_Address) override;
};

#endif
