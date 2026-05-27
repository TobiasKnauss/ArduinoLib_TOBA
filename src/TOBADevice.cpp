#include "TOBADevice.h"
#include "TOBADeviceConfig.h"
#include "TOBAWorker.h"
#include "TOBAWorker_CustomIO.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
::EResult TOBADevice::Create (Stream*           i_pCommStream,
                              UCOP*             i_pUCOP,
                              TOBADeviceConfig* i_pConfig,
                              TOBADevice*&      o_pDevice)
{
  o_pDevice = nullptr;
  if (i_pConfig == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  TOBADevice* pDevice = nullptr;
  ::EResult result = CreateObject (i_pConfig->get_DeviceType (), pDevice);
  if (result != ::EResult::SUCCESS)
    return result;

  result = pDevice->Init (i_pCommStream, i_pUCOP, i_pConfig);
  if (result != ::EResult::SUCCESS)
  {
    delete pDevice;
    return result;
  }

  result = pDevice->CreateDevices ();
  if (result != ::EResult::SUCCESS)
  {
    delete pDevice;
    return result;
  }

  o_pDevice = pDevice;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBADevice::~TOBADevice ()
{
  Clear ();

  if (m_NeedToDeleteUCOP)
  {
    DeleteObject (m_pUCOP);
    DeleteObject (m_pUCOPConfig);
  }

  DeleteObject (m_pReceiveBuffer);
  DeleteObject (m_pSendBuffer);
  DeleteObject (m_pPayloadRecvBuffer);
  DeleteObject (m_pPayloadSendBuffer);
}

//--------------------------------------------------------------------
TOBADevice::TOBADevice ()
{
}

//--------------------------------------------------------------------
::EResult TOBADevice::CreateObject (TOBADevice::EDeviceType i_Type,
                                    TOBADevice*&            o_pDevice)
{
  if (o_pDevice != nullptr)
    return ::EResult::FAIL_Pointer_IsNotZero;

  switch (i_Type)
  {
  case TOBADevice::EDeviceType::Worker:          o_pDevice = new TOBAWorker          (); break;
  case TOBADevice::EDeviceType::Worker_CustomIO: o_pDevice = new TOBAWorker_CustomIO (); break;
  default: return (::EResult)TOBA::EResult::FAIL_TOBA_DeviceTypeInvalid;
  }

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
bool TOBADevice::get_ExistsData ()
{
  return m_DataAvailable;
}

//--------------------------------------------------------------------
bool TOBADevice::get_ExistsReply ()
{
  return !m_ReplyData.IsEmpty ();
}

//--------------------------------------------------------------------
bool TOBADevice::get_ExistsRequest ()
{
  return !m_RequestData.IsEmpty ();
}

//--------------------------------------------------------------------
char* TOBADevice::get_DeviceName ()
{
  return m_pConfig->get_DeviceName ();
}

//--------------------------------------------------------------------
uint8_t TOBADevice::get_DeviceNameLength ()
{
  return m_pConfig->get_DeviceNameLength ();
}

//--------------------------------------------------------------------
TOBADevice::EDeviceType TOBADevice::get_DeviceType ()
{
  return EDeviceType::Worker;
}

//--------------------------------------------------------------------
void TOBADevice::Clear ()
{
  ClearBuffers ();
}

//--------------------------------------------------------------------
void TOBADevice::ClearBuffers ()
{
  memset (m_pReceiveBuffer    , c_BufferDefaultValue, m_pConfig->get_ReceiveBufferSize () );
  memset (m_pSendBuffer       , c_BufferDefaultValue, m_pConfig->get_SendBufferSize ()    );
  memset (m_pPayloadRecvBuffer, c_BufferDefaultValue, m_pConfig->get_PayloadBuffersSize ());
  memset (m_pPayloadSendBuffer, c_BufferDefaultValue, m_pConfig->get_PayloadBuffersSize ());
}

//--------------------------------------------------------------------
uint32_t TOBADevice::GetTimestamp ()
{
  return 0;
}

//--------------------------------------------------------------------
::EResult TOBADevice::ReadData ()
{
  if (m_pCommStream == nullptr)
    return (::EResult)TOBA::EResult::FAIL_TOBA_CommStream_NotReady;
  if (!m_pCommStream->available ())
    return ::EResult::SUCCESS;

  #ifdef TOBA_DEBUG
  Serial << F("CommStream available: ") << m_pCommStream->available () << endl;
  #endif

  // Receive all available data
  while (m_pCommStream->available ())
  {
    m_pReceiveBuffer[m_ReceiveBufferWriteIndex++] = m_pCommStream->read ();
    if (m_ReceiveBufferWriteIndex >= m_pConfig->get_ReceiveBufferSize ())
      m_ReceiveBufferWriteIndex = 0;
  }

  #ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer: position = ") << m_ReceiveBufferWriteIndex << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_pConfig->get_ReceiveBufferSize ());
  #endif

  m_DataAvailable = true;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBADevice::CreateDevices ()
{
  ::EResult result;

  // Create UCOP if needed
  if (m_pUCOP == nullptr)
  {
    UCOPConfig* pUCOPConfig = nullptr;
    result = UCOPConfig::Create (m_pConfig->get_EepromAddress_UCOPConfig (), pUCOPConfig);
    #ifdef TOBA_DEBUG
    Serial << F("UCOPConfig.Create() result=") << UCOP::GetResultText (result) << endl;
    #endif
    if (result != ::EResult::SUCCESS)
      return result;

    UCOP* pUCOP = nullptr;
    result = UCOP::Create (pUCOPConfig, pUCOP);;
    #ifdef TOBA_DEBUG
    Serial << F("UCOP.Create() result=") << UCOP::GetResultText (result) << endl;
    #endif
    if (result != ::EResult::SUCCESS)
    {
      delete pUCOPConfig;
      return result;
    }

    m_NeedToDeleteUCOP = true;
    m_pUCOPConfig = pUCOPConfig;
    m_pUCOP = pUCOP;
  }

  // Create buffers
  if (!Memory_Allocate (m_pReceiveBuffer    , m_pConfig->get_ReceiveBufferSize () , c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pSendBuffer       , m_pConfig->get_SendBufferSize ()    , c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pPayloadRecvBuffer, m_pConfig->get_PayloadBuffersSize (), c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;
  if (!Memory_Allocate (m_pPayloadSendBuffer, m_pConfig->get_PayloadBuffersSize (), c_BufferDefaultValue)) return ::EResult::FAIL_Buffer_Create;

  #ifdef TOBA_DEBUG
  Serial << F("ReceiveBuffer     Addr=") << _HEX4((uint16_t)m_pReceiveBuffer)     << F(" Len=") << m_pConfig->get_ReceiveBufferSize () << endl;
  Memory_PrintLn (m_pReceiveBuffer, m_pConfig->get_ReceiveBufferSize ());
  Serial << F("SendBuffer        Addr=") << _HEX4((uint16_t)m_pSendBuffer)        << F(" Len=") << m_pConfig->get_SendBufferSize () << endl;
  Memory_PrintLn (m_pSendBuffer, m_pConfig->get_SendBufferSize ());
  Serial << F("PayloadRecvBuffer Addr=") << _HEX4((uint16_t)m_pPayloadRecvBuffer) << F(" Len=") << m_pConfig->get_PayloadBuffersSize () << endl;
  Memory_PrintLn (m_pPayloadRecvBuffer, m_pConfig->get_PayloadBuffersSize ());
  Serial << F("PayloadSendBuffer Addr=") << _HEX4((uint16_t)m_pPayloadSendBuffer) << F(" Len=") << m_pConfig->get_PayloadBuffersSize () << endl;
  Memory_PrintLn (m_pPayloadSendBuffer, m_pConfig->get_PayloadBuffersSize ());
  #endif

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBADevice::Init (Stream*           i_pCommStream,
                            UCOP*             i_pUCOP,
                            TOBADeviceConfig* i_pConfig)
{
  if (i_pCommStream == nullptr
  ||  i_pConfig     == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  m_pCommStream = i_pCommStream;
  m_pUCOP       = i_pUCOP;
  m_pConfig     = i_pConfig;

  return ::EResult::SUCCESS;
}
