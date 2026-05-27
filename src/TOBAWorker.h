#ifndef TOBAWorker_h
#define TOBAWorker_h

#include "TOBADevice.h"

class TOBAWorkerConfig;

//--------------------------------------------------------------------
class TOBAWorker
: public TOBADevice
{
//==================== Fields ====================
private:
  //-------------------- instance --------------------

  TOBAWorkerConfig* m_pConfig = nullptr;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( Stream*           i_pCommStream,
                            UCOP*             i_pUCOP,
                            TOBAWorkerConfig* i_pConfig,
                            TOBAWorker*&      o_pDevice);

  //-------------------- instance --------------------

  TOBAWorker ();

  virtual ~TOBAWorker ();

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  bool get_ExistsWork ();

  bool get_IsBusy ();
  bool get_IsWorking ();

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  // Analyze the read data to find a request message.
  // The analysis is only allowed, if no active request, work, or reply exists; otherwise, an active request would be overwritten. The method contains checks to avoid that.
  // The analysis is only possible, if data exists. The method contains a check to ensure that.
  // If the analysis succeeds, a request is created. If the analysis fails, a failure reply is created.
  ::EResult AnalyzeData ();

  // Analyse the received request to retrieve a worker command.
  // The analysis is only allowed, if no active worker command or reply exists; otherwise, an active worker command would be overwritten. The method contains checks to avoid that.
  // It usually is sufficient to check if a request exists, because one was only created if the worker was not busy.
  // The analysis is only possible, if a request exists. The method contains a check to ensure that.
  // If the analysis succeeds, a worker command is created. If the analysis fails, a failure reply is created.
  ::EResult AnalyzeRequest ();

  // Clear all buffers, clear request and reply, and delete the worker command.
  virtual void Clear () override;

  // Clear request and reply, and delete the worker command.
  void ClearReqRepWoco ();

  // Send the reply that was created during analysis or work.
  // The sending is only allowed, if no active worker command exists; otherwise, a reply had been created early. The method contains a check to avoid that.
  // It usually is sufficient to check if a reply exists, because one was only created if the worker has finished or a failure happened.
  // The sending is only possible, if a reply exists. The method contains a check to ensure that.
  // The reply is deleted after it has been sent.
  ::EResult SendReply ();

  // Execute the received worker command.
  // The work is only allowed, if no active reply exists; otherwise, an active reply would be overwritten. The method contains a check to avoid that.
  // It usually is sufficient to check if a worker command exists, because one was only created if the worker was not busy.
  // The work is only possible, if a worker command exists. The method contains a check to ensure that.
  // When the work is done, the request and the worker command are deleted and a reply is created.
  virtual ::EResult Work ();

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult Init (Stream*           i_pCommStream,
                          UCOP*             i_pUCOP,
                          TOBADeviceConfig* i_pConfig) override;
};

#endif
