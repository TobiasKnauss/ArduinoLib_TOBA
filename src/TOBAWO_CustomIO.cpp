#include "TOBAWO_CustomIO.h"
#include "TOBA_defines.h"

#include <WOCO.h>
#include <WOCO_DigitalPinState.h>
#include <WOCO_DigitalPinMode.h>
#include <IOHelper.h>

//--------------------------------------------------------------------
TOBAWO_CustomIO::TOBAWO_CustomIO (Stream*    i_CommStream,
                                  uint16_t   i_ReceiveBufferSize,
                                  uint16_t   i_SendBufferSize,
                                  uint16_t   i_PayloadBuffersSize,
                                  ::EResult& o_Result)
: TOBA_Worker (i_CommStream,
               i_ReceiveBufferSize,
               i_SendBufferSize,
               i_PayloadBuffersSize,
               o_Result)
{
  if (o_Result != ::EResult::SUCCESS)
    return;
}

//--------------------------------------------------------------------
::EResult TOBAWO_CustomIO::Work ()
{
  ::EResult result = TOBA_Worker::Work ();
  if (result != ::EResult::InProgress)
  {
    DeleteObject (m_pWOCO);
    return result;
  }

  UCOP::EMessageResult messageResult = UCOP::EMessageResult::InProgress;

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
