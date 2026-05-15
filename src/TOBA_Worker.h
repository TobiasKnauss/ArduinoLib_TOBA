#ifndef TOBA_Worker_h
#define TOBA_Worker_h

#include <Result.h>
#include <MemoryTools.h>
#include <UCOP.h>
#include <UCOPData.h>
#include <WOCO.h>
#include <WOCO_AliveCheck.h>

//--------------------------------------------------------------------
// TOBA: TObi's Building Automation
//--------------------------------------------------------------------
class TOBA_Worker
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

//==================== Fields ====================
protected:
  UCOP*     m_pUCOP;
  UCOPData  m_RequestData;
  UCOPData  m_ReplyData;
  WOCO*     m_pWOCO;  // WOrker COmmand
 // WOCO*     m_pWORE;  // WOrker REply

private:
  const uint8_t c_MinRecvSendBuffersSize        = 40;
  const uint8_t c_MinPayloadRecvSendBuffersSize = 10;

  const uint16_t c_EepromAddr_WorkerConfig = 0;
  const uint16_t c_EepromAddr_UcopConfig = 10;
  const uint8_t c_BufferDefaultValue = 0xFF;

  static const char* const c_EResult_ClassFailures_Names[] PROGMEM;

  #define X(name) static const char _EResult_##name[] PROGMEM;
  #include "TOBA_EResult_failures.h"
  #undef X

  uint8_t* m_pPayloadRecvBuffer;
  uint8_t* m_pPayloadSendBuffer;
  uint8_t* m_pReceiveBuffer;
  uint8_t* m_pSendBuffer;
  uint16_t m_ReceiveBufferSize  = 0;
  uint16_t m_SendBufferSize     = 0;
  uint16_t m_PayloadBuffersSize = 0;

  uint16_t m_ReceiveBufferWriteIndex = 0;
  uint16_t m_ReceiveBufferReadIndex  = 0;

  Stream* m_pCommStream;

  bool m_DataAvailable = false;
  bool m_IsWorking     = false;

//==================== Constructors ====================
public:
  TOBA_Worker (Stream*    i_pCommStream,
               uint16_t   i_ReceiveBufferSize,
               uint16_t   i_SendBufferSize,
               uint16_t   i_PayloadBuffersSize,
               ::EResult& o_Result);

//==================== Properties ====================
public:
  bool get_IsWorking ();

//==================== Public Methods ====================
public:
  //-------------------- static --------------------

  static const __FlashStringHelper* GetResultText (::EResult i_Result);

  //-------------------- instance --------------------

  ::EResult AnalyzeData ();

  ::EResult AnalyzeRequest ();

  ::EResult ReadData ();

  ::EResult SendReply ();

  virtual ::EResult Work ();

//==================== Public Methods ====================
private:
  //-------------------- instance --------------------

  void ClearReceiveBuffer ();
};

#endif
