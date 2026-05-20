#ifndef TOBAWO_CustomIO_h
#define TOBAWO_CustomIO_h

#include "TOBA_Worker.h"

class TOBAConfig_CustomIO;

//--------------------------------------------------------------------
class TOBAWO_CustomIO
: public TOBA_Worker
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  TOBAConfig_CustomIO* m_pConfig = nullptr;

//==================== Constructors ====================
public:
  //-------------------- instance --------------------

  TOBAWO_CustomIO ( Stream*             i_pCommStream,
                    UCOP*               i_pUCOP,
                    TOBAConfig_Worker*  i_pConfig,
                    ::EResult&          o_Result);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  virtual EWorkerType get_WorkerType () override;

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  virtual ::EResult Work () override;
};

#endif
