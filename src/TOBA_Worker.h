#ifndef TOBA_Worker_h
#define TOBA_Worker_h

#include <FastCRC.h>
#include <Result.h>
#include <MemoryTools.h>
#include <UCOP.h>
#include <UCOPData.h>
#include <WOCO.h>
#include <WOCO_AliveCheck.h>
#include <WOCO_WorkerName.h>
#include <WOCO_WorkerType.h>

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

  enum class EWorkerType : uint32_t
  {
    None = 0,
    BuiltIn_Basic = 0x0100,
    BuiltIn_CustomIO = 0x0101,
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

  static const uint8_t c_EepromConfigDataSize          = 40;
  static const uint8_t c_EepromConfigTotalSize         = 42;
  static const uint8_t c_MinRecvSendBuffersSize        = 40;
  static const uint8_t c_MinPayloadRecvSendBuffersSize = 10;

  static const uint8_t c_BufferDefaultValue = 0xFF;

  static const char* const c_EResult_ClassFailures_Names[] PROGMEM;

  #define X(name) static const char _EResult_##name[] PROGMEM;
  #include "TOBA_EResult_failures.h"
  #undef X

  //-------------------- instance --------------------

  uint8_t* m_pReceiveBuffer     = nullptr;
  uint8_t* m_pSendBuffer        = nullptr;
  uint16_t m_SendBufferSize     = 0;
  uint16_t m_ReceiveBufferSize  = 0;

  bool      m_NeedToDeleteUCOP  = false;
  uint16_t  m_EepromAddress     = 0;
  char      m_WorkerName[32];
  Stream*   m_pCommStream = nullptr;
  FastCRC16 m_Crc16;

  uint16_t m_ReceiveBufferWriteIndex = 0;
  uint16_t m_ReceiveBufferReadIndex  = 0;
  bool     m_DataAvailable           = false;

//==================== Constructors ====================
public:
  //-------------------- instance --------------------

  TOBA_Worker ( Stream*    i_pCommStream,
                uint16_t   i_ReceiveBufferSize,
                uint16_t   i_SendBufferSize,
                uint16_t   i_PayloadBuffersSize,
                char*      i_pWorkerName,
                uint8_t    i_WorkerNameLength,
                UCOP*      i_pUCOP,
                ::EResult& o_Result);

  TOBA_Worker ( Stream*    i_pCommStream,
                uint16_t   i_EepromAddress,
                ::EResult& o_Result);

  ~TOBA_Worker ();

private:
  //-------------------- instance --------------------

  ::EResult CommonConstructor_Cfg ( uint16_t i_ReceiveBufferSize,
                                    uint16_t i_SendBufferSize,
                                    uint16_t i_PayloadBuffersSize,
                                    char*    i_pWorkerName,
                                    uint8_t  i_WorkerNameLength,
                                    UCOP*    i_pUCOP);

  ::EResult CommonConstructor_Ext (Stream* i_pCommStream);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  uint16_t get_EepromAddress ();

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

  ::EResult WriteConfigToEEPROM (uint16_t i_Address);

//==================== Private Methods ====================
private:
  //-------------------- instance --------------------

  ::EResult ReadConfigFromEEPROM (uint16_t i_Address);
};

#endif
