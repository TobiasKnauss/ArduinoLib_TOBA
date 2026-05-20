#include "TOBAConfig_CustomIO.h"

//--------------------------------------------------------------------
::EResult TOBAConfig_CustomIO::Create ( uint16_t              i_ReceiveBufferSize,
                                        uint16_t              i_SendBufferSize,
                                        uint16_t              i_PayloadBuffersSize,
                                        char*                 i_pWorkerName,
                                        uint8_t               i_WorkerNameLength,
                                        uint16_t              i_EepromAddress_UCOPConfig,
                                        uint8_t               i_IOCount,
                                        uint8_t*              i_pIOPins,
                                        TOBAConfig_CustomIO*& o_pConfig)
{
  o_pConfig = nullptr;
  TOBAConfig_CustomIO* pConfig = new TOBAConfig_CustomIO (i_ReceiveBufferSize,
                                                          i_SendBufferSize,
                                                          i_PayloadBuffersSize,
                                                          i_pWorkerName,
                                                          i_WorkerNameLength,
                                                          i_EepromAddress_UCOPConfig,
                                                          i_IOCount,
                                                          i_pIOPins);
  ::EResult result = pConfig->VerifyConfig_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete (pConfig);
    return result;
  }

  o_pConfig = pConfig;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBAConfig_CustomIO::TOBAConfig_CustomIO ()
{
}

//--------------------------------------------------------------------
TOBAConfig_CustomIO::~TOBAConfig_CustomIO ()
{
  delete m_pIOPins;
}

//--------------------------------------------------------------------
TOBAConfig_CustomIO::TOBAConfig_CustomIO (uint16_t    i_ReceiveBufferSize,
                                          uint16_t    i_SendBufferSize,
                                          uint16_t    i_PayloadBuffersSize,
                                          char*       i_pWorkerName,
                                          uint8_t     i_WorkerNameLength,
                                          uint16_t    i_EepromAddress_UCOPConfig,
                                          uint8_t     i_IOCount,
                                          uint8_t*    i_pIOPins)
: TOBAConfig_Worker ( i_ReceiveBufferSize,
                      i_SendBufferSize,
                      i_PayloadBuffersSize,
                      i_pWorkerName,
                      i_WorkerNameLength,
                      i_EepromAddress_UCOPConfig)
{
  m_IOCount = i_IOCount;

  if (i_pIOPins != nullptr)
  {
    m_pIOPins  = new uint8_t[i_IOCount];
    memcpy (m_pIOPins, i_pIOPins, i_IOCount);
  }
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_CustomIO::get_EepromConfigDataSize ()
{
  return TOBAConfig_Worker::get_EepromConfigDataSize ()
       + 1
       + m_IOCount;
}

//--------------------------------------------------------------------
uint8_t TOBAConfig_CustomIO::get_IOCount ()
{
  return m_IOCount;
}

//--------------------------------------------------------------------
uint8_t* TOBAConfig_CustomIO::get_IOPins ()
{
  return m_pIOPins;
}

//--------------------------------------------------------------------
TOBA_Worker::EWorkerType TOBAConfig_CustomIO::get_WorkerType ()
{
  return EWorkerType::BuiltIn_CustomIO;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_CustomIO::ReadConfigFromEEPROM_EXEC (uint16_t& io_Address)
{
  ::EResult result = TOBAConfig_Worker::ReadConfigFromEEPROM_EXEC (io_Address);
  if (result != ::EResult::SUCCESS)
    return result;

  bool isOK = true;
  isOK &= EEPROM_GetValueAndMovePtr (io_Address, m_IOCount);
  delete (m_pIOPins);
  m_pIOPins = new uint8_t[m_IOCount];
  isOK &= EEPROM_GetBytesAndMovePtr (io_Address, m_IOCount, m_pIOPins);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_CustomIO::VerifyConfig_EXEC ()
{
  ::EResult result = TOBAConfig_Worker::VerifyConfig_EXEC ();
  if (result != ::EResult::SUCCESS)
    return result;

  if (m_pIOPins == nullptr)
    return ::EResult::FAIL_Pointer_IsZero;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAConfig_CustomIO::WriteConfigToEEPROM_EXEC (uint16_t& io_Address)
{
  ::EResult result = TOBAConfig_Worker::WriteConfigToEEPROM_EXEC (io_Address);
  if (result != ::EResult::SUCCESS)
    return result;

  bool isOK = true;
  isOK &= EEPROM_SetValueAndMovePtr (io_Address, m_IOCount);
  isOK &= EEPROM_SetBytesAndMovePtr (io_Address, m_IOCount, m_pIOPins);
  if (!isOK)
    return ::EResult::FAIL_EEPROM_GetValue;

  return ::EResult::SUCCESS;
}
