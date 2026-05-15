#include "TOBAWO_CustomIO.h"

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
  ::EResult result = ::EResult::InProgress;

  WOCO* pWORE = 0;  // Reply
  switch (m_pWOCO->GetCommand ())
  {
  case WOCO::ECommand::DigitalPinMode:
    {
      WOCO_DigitalPinMode* pWOCO_DigitalPinMode = (WOCO_DigitalPinMode*)m_pWOCO;
      if (pWOCO_DigitalPinMode->GetActionIsWrite ())
      {
        pinMode (pWOCO_DigitalPinMode->GetPinNumber (),
                  pWOCO_DigitalPinMode->GetPinMode ());
        pWORE = WOCO_DigitalPinMode::CreateWriteReply ();
      }
      else
      {
        uint8_t currentPinMode = getPinMode (pWOCO_DigitalPinMode->GetPinNumber ());
        pWORE = WOCO_DigitalPinMode::CreateReadReply(pWOCO_DigitalPinMode->GetPinNumber (), currentPinMode);
      }
    }
    break;

  case WOCO::ECommand::DigitalPinState:
    {
      WOCO_DigitalPinState* pWOCO_DigitalPinState = (WOCO_DigitalPinState*)m_pWOCO;
      if (pWOCO_DigitalPinState->GetActionIsWrite ())
      {
        digitalWrite (pWOCO_DigitalPinState->GetPinNumber (),
                      pWOCO_DigitalPinState->GetPinState ());
      }
      else
      {
        bool ioState = digitalRead (pWOCO_DigitalPinState->GetPinNumber ());
        pWORE = WOCO_DigitalPinState::CreateReadReply (pWOCO_DigitalPinState->GetPinNumber (), ioState);
      }
    }
    break;

  default:
    result = TOBA_Worker::Work ();
    break;
  }

}