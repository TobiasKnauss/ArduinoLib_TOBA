#ifndef TOBAController_h
#define TOBAController_h

#include "TOBADevice.h"

class TOBAControllerConfig;

//--------------------------------------------------------------------
class TOBAController
: public TOBADevice
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  TOBAControllerConfig* m_pConfig = nullptr;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( Stream*               i_pCommStream,
                            UCOP*                 i_pUCOP,
                            TOBAControllerConfig* i_pConfig,
                            TOBAController*&      o_pDevice);

  //-------------------- instance --------------------

  TOBAController ();

  virtual ~TOBAController ();

//==================== Properties ====================
public:
  //-------------------- instance --------------------

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  // Analyze the read data to find a reply message.
  ::EResult AnalyzeData ();

  // Analyse the received reply message to retrieve a worker reply.
  ::EResult AnalyzeReply (WOCO*& o_pWORE);

  // Clear all buffers, clear request and reply, and delete the worker command.
  virtual void Clear () override;

  // Clear request and reply.
  void ClearReqRep ();

  // Create a request from the given worker command.
  ::EResult CreateRequest ( uint32_t  i_WorkerDeviceId,
                            WOCO*     i_pWOCO);

  // Send the request to the worker.
  ::EResult SendRequest ();

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult Init (Stream*           i_pCommStream,
                          UCOP*             i_pUCOP,
                          TOBADeviceConfig* i_pConfig) override;
};

#endif
