#ifndef TOBAWorker_Basic_h
#define TOBAWorker_Basic_h

#include <FastCRC.h>
#include <Result.h>
#include <MemoryTools.h>
#include <MemoryTools_RingBuffer.h>
#include <UCOP.h>
#include <UCOPData.h>
#include <WOCO.h>

class TOBAConfig_Basic;

//--------------------------------------------------------------------
// TOBA: TObi's Building Automation
//--------------------------------------------------------------------
class TOBAWorker_Basic
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
    BuiltIn_Basic    = 0x00000100,
    BuiltIn_CustomIO = 0x00000101,
  };

//==================== Fields ====================
protected:
  //-------------------- instance --------------------

  UCOP*     m_pUCOP = nullptr;  // Universal Communication Protocol
  UCOPData  m_RequestData;
  UCOPData  m_ReplyData;
  WOCO*     m_pWOCO = nullptr;  // WOrker COmmand

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

  TOBAConfig_Basic* m_pConfig = nullptr;
  
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

  static ::EResult Create ( Stream*             i_pCommStream,
                            UCOP*               i_pUCOP,
                            TOBAConfig_Basic*   i_pConfig,
                            TOBAWorker_Basic*&  o_pWorker);

  //-------------------- instance --------------------

  virtual ~TOBAWorker_Basic ();

protected:
  //-------------------- instance --------------------

  TOBAWorker_Basic (Stream*           i_pCommStream,
                    UCOP*             i_pUCOP,
                    TOBAConfig_Basic* i_pConfig);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  bool get_ExistsReply ();

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

  ::EResult AnalyzeData ();

  ::EResult AnalyzeRequest ();

  uint32_t GetTimestamp ();

  ::EResult ReadData ();

  ::EResult SendReply ();

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
