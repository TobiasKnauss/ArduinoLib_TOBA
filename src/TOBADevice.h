#ifndef TOBADevice_h
#define TOBADevice_h

#include <FastCRC.h>
#include <MemoryTools.h>
#include <MemoryTools_RingBuffer.h>
#include <UCOP.h>
#include <UCOPConfig.h>
#include <UCOPData.h>
#include <WOCO.h>

#include "TOBA.h"

class TOBADeviceConfig;

//--------------------------------------------------------------------
class TOBADevice
{
//==================== Enums ====================
public:
  enum class EDeviceType : uint32_t
  {
    None = 0,
    Controller      = 0x00000001,
    Worker          = 0x00000100,
    Worker_CustomIO = 0x00000101,
  };

//==================== Fields ====================
protected:
  //-------------------- static --------------------

  static const uint8_t c_BufferDefaultValue = 0xFF;

  //-------------------- instance --------------------

  Stream*     m_pCommStream = nullptr;
  UCOPConfig* m_pUCOPConfig = nullptr;
  UCOP*       m_pUCOP = nullptr;  // Universal Communication Protocol
  UCOPData    m_RequestData;
  UCOPData    m_ReplyData;
  WOCO*       m_pWOCO = nullptr;  // WOrker COmmand

  uint8_t* m_pReceiveBuffer     = nullptr;
  uint8_t* m_pSendBuffer        = nullptr;
  uint8_t* m_pPayloadRecvBuffer = nullptr;
  uint8_t* m_pPayloadSendBuffer = nullptr;
  uint16_t m_PayloadBuffersSize = 0;

  uint16_t m_ReceiveBufferWriteIndex = 0;
  uint16_t m_ReceiveBufferReadIndex  = 0;
  bool     m_DataAvailable           = false;

private:
  //-------------------- instance --------------------

  TOBADeviceConfig* m_pConfig = nullptr;

  bool m_NeedToDeleteUCOP = false;

//==================== Constructors ====================
public:
  //-------------------- static --------------------

  static ::EResult Create ( Stream*           i_pCommStream,
                            UCOP*             i_pUCOP,
                            TOBADeviceConfig* i_pConfig,
                            TOBADevice*&      o_pDevice);

  //-------------------- instance --------------------

  virtual ~TOBADevice ();

protected:
  //-------------------- instance --------------------

  TOBADevice ();

private:
  //-------------------- static --------------------

  static ::EResult CreateObject ( TOBADevice::EDeviceType i_Type,
                                  TOBADevice*&            o_pDevice);

//==================== Properties ====================
public:
  //-------------------- instance --------------------

  bool get_ExistsData ();
  bool get_ExistsRequest ();
  bool get_ExistsReply ();

  char*   get_DeviceName ();
  uint8_t get_DeviceNameLength ();

  virtual EDeviceType get_DeviceType ();

//==================== Public Methods ====================
public:
  //-------------------- instance --------------------

  // Clear all buffers, clear request and reply, and delete the worker command.
  virtual void Clear ();

  // Clear all buffers.
  void ClearBuffers ();

  uint32_t GetTimestamp ();

  // Read data from the stored stream and write it into a buffer.
  // The reading is always allowed and possible.
  ::EResult ReadData ();

//==================== Protected Methods ====================
protected:
  //-------------------- instance --------------------

  virtual ::EResult CreateDevices ();

  virtual ::EResult Init (Stream*           i_pCommStream,
                          UCOP*             i_pUCOP,
                          TOBADeviceConfig* i_pConfig);
};

#endif
