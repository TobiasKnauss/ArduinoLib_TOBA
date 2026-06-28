#include <WOCO_DigitalIOState.h>
#include <WOCO_DigitalIOMode.h>
#include <IOHelper.h>

#include "TOBAWorker_CustomIO.h"
#include "TOBAWorkerConfig_CustomIO.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
TOBAWorker_CustomIO::TOBAWorker_CustomIO ()
: TOBAWorker ()
{
}

//--------------------------------------------------------------------
TOBAWorker_CustomIO::~TOBAWorker_CustomIO ()
{
}

//--------------------------------------------------------------------
TOBADevice::EDeviceType TOBAWorker_CustomIO::get_DeviceType ()
{
  return EDeviceType::Worker_CustomIO;
}

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Init ( Stream*           i_pCommStream,
                                      UCOP*             i_pUCOP,
                                      TOBADeviceConfig* i_pConfig)
{
  ::EResult result = TOBADevice::Init (i_pCommStream, i_pUCOP, i_pConfig);
  if (result != ::EResult::SUCCESS)
    return result;

  m_pConfig = (TOBAWorkerConfig_CustomIO*)i_pConfig;

  return ::EResult::SUCCESS;
}

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Work_EXEC (WOCO*&                o_pWORE,  // WOrker REply
                                          UCOP::EMessageResult& o_MessageResult)
{
  ::EResult result = TOBAWorker::Work_EXEC (o_pWORE, o_MessageResult);
  if (result != ::EResult::InProgress)
    return result;

  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::DigitalIOMode:
    {
      WOCO_DigitalIOMode* pWOCO_DigitalIOMode = (WOCO_DigitalIOMode*)m_pWOCO;
      uint8_t pinNumber = m_pConfig->get_IOPin (pWOCO_DigitalIOMode->get_IONumber ());
      if (pinNumber > 0)
      {
        if (pWOCO_DigitalIOMode->get_ActionIsWrite ())
        {
          pinMode (pinNumber, pWOCO_DigitalIOMode->get_IOMode ());
          o_pWORE = WOCO_DigitalIOMode::CreateWriteReply ();
        }
        else
        {
          uint8_t currentIOMode = getPinMode (pinNumber);
          o_pWORE = WOCO_DigitalIOMode::CreateReadReply(pWOCO_DigitalIOMode->get_IONumber (), currentIOMode);
        }
        o_MessageResult = UCOP::EMessageResult::SUCCESS;
      }
      else
      {
        o_MessageResult = UCOP::EMessageResult::FAIL_CommandExecutionFailed;
      }
    }
    break;

  case WOCO::ECommand::DigitalIOState:
    {
      WOCO_DigitalIOState* pWOCO_DigitalIOState = (WOCO_DigitalIOState*)m_pWOCO;
      uint8_t pinNumber = m_pConfig->get_IOPin (pWOCO_DigitalIOState->get_IONumber ());
      if (pinNumber > 0)
      {
        if (pWOCO_DigitalIOState->get_ActionIsWrite ())
        {
          digitalWrite (pinNumber, pWOCO_DigitalIOState->get_IOState ());
          o_pWORE = WOCO_DigitalIOState::CreateWriteReply ();
        }
        else
        {
          bool ioState = digitalRead (pinNumber);
          o_pWORE = WOCO_DigitalIOState::CreateReadReply (pWOCO_DigitalIOState->get_IONumber (), ioState);
        }
        o_MessageResult = UCOP::EMessageResult::SUCCESS;
      }
      else
      {
        o_MessageResult = UCOP::EMessageResult::FAIL_CommandExecutionFailed;
      }
    }
    break;

  default:
    o_MessageResult = UCOP::EMessageResult::FAIL_CommandNotSupported;
    break;
  }

  return ::EResult::SUCCESS;
}
