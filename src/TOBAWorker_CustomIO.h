#ifndef TOBAWorker_CustomIO_h
#define TOBAWorker_CustomIO_h

#include "TOBAWorker_Basic.h"

class TOBAConfig_CustomIO;

//--------------------------------------------------------------------
class TOBAWorker_CustomIO
: public TOBAWorker_Basic
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  TOBAConfig_CustomIO* m_pConfig = nullptr;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( Stream*               i_pCommStream,
                            UCOP*                 i_pUCOP,
                            TOBAConfig_Basic*     i_pConfig,
                            TOBAWorker_CustomIO*& o_pWorker);

  //-------------------- instance --------------------

  ~TOBAWorker_CustomIO ();

protected:
  //-------------------- instance --------------------

  TOBAWorker_CustomIO ( Stream*           i_pCommStream,
                        UCOP*             i_pUCOP,
                        TOBAConfig_Basic* i_pConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual EWorkerType get_WorkerType () override;

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  virtual ::EResult Work () override;

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult CreateDevices_EXEC () override;

  virtual ::EResult Verify_EXEC () override;

//==================== Private Methods ====================
private:
  //-------------------- instance --------------------
};

#endif
