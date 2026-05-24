#include <WOCO_DigitalPinState.h>
#include <WOCO_DigitalPinMode.h>
#include <IOHelper.h>

#include "TOBAWorker_CustomIO.h"
#include "TOBAConfig_CustomIO.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Create ( Stream*               i_pCommStream,
                                        UCOP*                 i_pUCOP,
                                        TOBAConfig_Basic*     i_pConfig,
                                        TOBAWorker_CustomIO*& o_pWorker)
{
  o_pWorker = nullptr;
  TOBAWorker_CustomIO* pWorker = new TOBAWorker_CustomIO (i_pCommStream,
                                                          i_pUCOP,
                                                          i_pConfig);

  ::EResult result = pWorker->Verify_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete pWorker;
    return result;
  }

  result = pWorker->CreateDevices_EXEC ();
  if (result != ::EResult::SUCCESS)
  {
    delete pWorker;
    return result;
  }

  o_pWorker = pWorker;
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
TOBAWorker_CustomIO::~TOBAWorker_CustomIO ()
{
}

//--------------------------------------------------------------------
TOBAWorker_CustomIO::TOBAWorker_CustomIO (Stream*           i_pCommStream,
                                          UCOP*             i_pUCOP,
                                          TOBAConfig_Basic* i_pConfig)
: TOBAWorker_Basic (i_pCommStream,
                    i_pUCOP,
                    i_pConfig)
{
  m_pConfig = (TOBAConfig_CustomIO*)i_pConfig;
}

//--------------------------------------------------------------------
TOBAWorker_Basic::EWorkerType TOBAWorker_CustomIO::get_WorkerType ()
{
  return EWorkerType::BuiltIn_CustomIO;
}

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Work ()
{
  ::EResult result = TOBAWorker_Basic::Work ();
  if (result != ::EResult::InProgress)
  {
    DeleteObject (m_pWOCO);
    return result;
  }

  UCOP::EMessageResult messageResult = UCOP::EMessageResult::InProgress;

  uint8_t ioPin = 0;
  WOCO* pWORE = nullptr;  // WOrker REply
  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::DigitalPinMode:
    {
      WOCO_DigitalPinMode* pWOCO_DigitalPinMode = (WOCO_DigitalPinMode*)m_pWOCO;
      if (pWOCO_DigitalPinMode->get_ActionIsWrite ())
      {
        pinMode (pWOCO_DigitalPinMode->get_PinNumber (),
                  pWOCO_DigitalPinMode->get_PinMode ());
        pWORE = WOCO_DigitalPinMode::CreateWriteReply ();
      }
      else
      {
        uint8_t currentPinMode = getPinMode (pWOCO_DigitalPinMode->get_PinNumber ());
        pWORE = WOCO_DigitalPinMode::CreateReadReply(pWOCO_DigitalPinMode->get_PinNumber (), currentPinMode);
      }
      messageResult = UCOP::EMessageResult::SUCCESS;
    }
    break;

  case WOCO::ECommand::DigitalPinState:
    {
      WOCO_DigitalPinState* pWOCO_DigitalPinState = (WOCO_DigitalPinState*)m_pWOCO;
      if (pWOCO_DigitalPinState->get_ActionIsWrite ())
      {
        digitalWrite (pWOCO_DigitalPinState->get_PinNumber (),
                      pWOCO_DigitalPinState->get_PinState ());
        pWORE = WOCO_DigitalPinState::CreateWriteReply ();
      }
      else
      {
        bool ioState = digitalRead (pWOCO_DigitalPinState->get_PinNumber ());
        pWORE = WOCO_DigitalPinState::CreateReadReply (pWOCO_DigitalPinState->get_PinNumber (), ioState);
      }
      messageResult = UCOP::EMessageResult::SUCCESS;
    }
    break;

  default:
    m_ReplyData = UCOPData::Create_CommandNotSupported (m_RequestData, GetTimestamp ());
    break;
  }

  DeleteObject (m_pWOCO);

  if (pWORE != nullptr)
  {
    m_ReplyData = UCOPData (m_RequestData.ActionIsWrite,
                            m_RequestData.RemoteDeviceId,
                            m_RequestData.MessageId,
                            GetTimestamp (),
                            m_RequestData.CommandId,
                            messageResult);
    m_ReplyData.SetPayloadInfo (m_pPayloadSendBuffer, m_PayloadBuffersSize);

    result = pWORE->ComposeCommandData (m_ReplyData.pPayloadBuffer,
                                        m_ReplyData.PayloadBufferLength,
                                        m_ReplyData.PayloadLength);

    DeleteObject (pWORE);
  }

  if (m_ReplyData.IsEmpty ())
    return (::EResult)EResult::FAIL_TOBA_ReplyMissing;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::CreateDevices_EXEC ()
{
  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Verify_EXEC ()
{
  return ::EResult::SUCCESS;
}
