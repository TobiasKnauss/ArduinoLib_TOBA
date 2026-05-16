#ifndef TOBAWO_CustomIO_h
#define TOBAWO_CustomIO_h

#include "TOBA_Worker.h"

//--------------------------------------------------------------------
class TOBAWO_CustomIO
: public TOBA_Worker
{
//==================== Constructors ====================
public:
  //-------------------- instance --------------------

  TOBAWO_CustomIO (Stream*    i_CommStream,
                   uint16_t   i_ReceiveBufferSize,
                   uint16_t   i_SendBufferSize,
                   uint16_t   i_PayloadBuffersSize,
                   ::EResult& o_Result);

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  virtual ::EResult Work () override;
};

#endif
