#ifndef TOBAWorker_h
#define TOBAWorker_h

#include <FastCRC.h>
#include <Result.h>
#include <MemoryTools.h>
#include <MemoryTools_RingBuffer.h>
#include <UCOP.h>
#include <UCOPConfig.h>
#include <UCOPData.h>
#include <WOCO.h>

class TOBAConfig;

//--------------------------------------------------------------------
// TOBA: TObi's Building Automation
//--------------------------------------------------------------------
class TOBAWorker
{
//==================== Enums ====================
public:
  enum class EResult : uint16_t
  {
    Dummy_FirstClassFailure = (uint16_t)::EResult::Dummy_FirstClassFailure,
    #define X(name) name,
    #include "TOBA_EResult_failures.h"
    #undef X
    Dummy_LastClassFailure
  };

  enum class EWorkerType : uint32_t
  {
    None = 0,
    BuiltIn          = 0x00000100,
    BuiltIn_CustomIO = 0x00000101,
  };

//==================== Fields ====================
protected:
  //-------------------- instance --------------------

  UCOPConfig* m_pUCOPConfig = nullptr;
  UCOP*       m_pUCOP = nullptr;  // Universal Communication Protocol
  UCOPData    m_RequestData;
  UCOPData    m_ReplyData;
  WOCO*       m_pWOCO = nullptr;  // WOrker COmmand

  uint8_t* m_pPayloadRecvBuffer = nullptr;
  uint8_t* m_pPayloadSendBuffer = nullptr;
  uint16_t m_PayloadBuffersSize = 0;

private:
  //-------------------- static --------------------

  static const uint8_t c_BufferDefaultValue = 0xFF;

  static const char* const c_EResult_ClassFailures_Names[] PROGMEM;

  #define X(name) static const char _EResult_##name[] PROGMEM;
  #include "TOBA_EResult_failures.h"
  #undef X

  //-------------------- instance --------------------

  TOBAConfig* m_pConfig = nullptr;

  uint8_t* m_pReceiveBuffer     = nullptr;
  uint8_t* m_pSendBuffer        = nullptr;

  Stream*   m_pCommStream = nullptr;
  bool      m_NeedToDeleteUCOP = false;
  FastCRC16 m_Crc16;

  uint16_t m_ReceiveBufferWriteIndex = 0;
  uint16_t m_ReceiveBufferReadIndex  = 0;
  bool     m_DataAvailable           = false;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( Stream*       i_pCommStream,
                            UCOP*         i_pUCOP,
                            TOBAConfig*   i_pConfig,
                            TOBAWorker*&  o_pWorker);

  //-------------------- instance --------------------

  virtual ~TOBAWorker ();

protected:
  //-------------------- instance --------------------

  TOBAWorker (Stream*     i_pCommStream,
              UCOP*       i_pUCOP,
              TOBAConfig* i_pConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  bool get_ExistsData ();
  bool get_ExistsRequest ();
  bool get_ExistsReply ();
  bool get_ExistsWork ();

  bool get_IsBusy ();
  bool get_IsWorking ();

  char*   get_WorkerName ();
  uint8_t get_WorkerNameLength ();

  virtual EWorkerType get_WorkerType ();

//==================== Public Methods ====================
public:
  //-------------------- static --------------------

  static const __FlashStringHelper* GetResultText (::EResult i_Result);

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
  void Clear ();

  // Clear all buffers.
  void ClearBuffers ();

  // Clear request and reply, and delete the worker command.
  void ClearReqRepWoco ();

  uint32_t GetTimestamp ();

  // Read data from the stored stream and write it into a buffer.
  // The reading is always allowed and possible.
  ::EResult ReadData ();

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

  virtual ::EResult CreateDevices_EXEC ();

  virtual ::EResult Verify_EXEC ();

//==================== Private Methods ====================
private:
  //-------------------- instance --------------------
};

#endif
