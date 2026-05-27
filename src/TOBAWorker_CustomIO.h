#ifndef TOBAWorker_CustomIO_h
#define TOBAWorker_CustomIO_h

#include "TOBAWorker.h"

class TOBAWorkerConfig_CustomIO;

//--------------------------------------------------------------------
class TOBAWorker_CustomIO
: public TOBAWorker
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  TOBAWorkerConfig_CustomIO* m_pConfig = nullptr;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( Stream*               i_pCommStream,
                            UCOP*                 i_pUCOP,
                            TOBAWorkerConfig*     i_pConfig,
                            TOBAWorker_CustomIO*& o_pWorker);

  //-------------------- instance --------------------

  TOBAWorker_CustomIO ();

  ~TOBAWorker_CustomIO ();

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual EDeviceType get_DeviceType () override;

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  virtual ::EResult Work () override;

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult Init (Stream*           i_pCommStream,
                          UCOP*             i_pUCOP,
                          TOBADeviceConfig* i_pConfig) override;
};

#endif
