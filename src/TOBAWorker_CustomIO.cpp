#include <WOCO_DigitalIOState.h>
#include <WOCO_DigitalIOMode.h>
#include <IOHelper.h>

#include "TOBAWorker_CustomIO.h"
#include "TOBAConfig_CustomIO.h"
#include "TOBA_defines.h"

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Create ( Stream*               i_pCommStream,
                                        UCOP*                 i_pUCOP,
                                        TOBAConfig*           i_pConfig,
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
TOBAWorker_CustomIO::TOBAWorker_CustomIO (Stream*     i_pCommStream,
                                          UCOP*       i_pUCOP,
                                          TOBAConfig* i_pConfig)
: TOBAWorker (i_pCommStream,
                    i_pUCOP,
                    i_pConfig)
{
  m_pConfig = (TOBAConfig_CustomIO*)i_pConfig;
}

//--------------------------------------------------------------------
TOBAWorker::EWorkerType TOBAWorker_CustomIO::get_WorkerType ()
{
  return EWorkerType::BuiltIn_CustomIO;
}

//--------------------------------------------------------------------
::EResult TOBAWorker_CustomIO::Work ()
{
  ::EResult result = TOBAWorker::Work ();
  if (result != ::EResult::InProgress)
  {
    DeleteObject (m_pWOCO);
    return result;
  }

  UCOP::EMessageResult messageResult = UCOP::EMessageResult::InProgress;

  uint8_t pinNumber = 0;
  WOCO* pWORE = nullptr;  // WOrker REply
  switch (m_pWOCO->get_Command ())
  {
  case WOCO::ECommand::DigitalIOMode:
    {
      WOCO_DigitalIOMode* pWOCO_DigitalIOMode = (WOCO_DigitalIOMode*)m_pWOCO;
      pinNumber = m_pConfig->get_IOPin (pWOCO_DigitalIOMode->get_IONumber ());
      if (pinNumber > 0)
      {
        if (pWOCO_DigitalIOMode->get_ActionIsWrite ())
        {
          pinMode (pinNumber, pWOCO_DigitalIOMode->get_IOMode ());
          pWORE = WOCO_DigitalIOMode::CreateWriteReply ();
        }
        else
        {
          uint8_t currentIOMode = getPinMode (pinNumber);
          pWORE = WOCO_DigitalIOMode::CreateReadReply(pWOCO_DigitalIOMode->get_IONumber (), currentIOMode);
        }
        messageResult = UCOP::EMessageResult::SUCCESS;
      }
      else
      {
        messageResult = UCOP::EMessageResult::FAIL_CommandExecutionFailed;
      }
    }
    break;

  case WOCO::ECommand::DigitalIOState:
    {
      WOCO_DigitalIOState* pWOCO_DigitalIOState = (WOCO_DigitalIOState*)m_pWOCO;
      pinNumber = m_pConfig->get_IOPin (pWOCO_DigitalIOState->get_IONumber ());
      if (pinNumber > 0)
      {
        if (pWOCO_DigitalIOState->get_ActionIsWrite ())
        {
          digitalWrite (pinNumber, pWOCO_DigitalIOState->get_IOState ());
          pWORE = WOCO_DigitalIOState::CreateWriteReply ();
        }
        else
        {
          bool ioState = digitalRead (pinNumber);
          pWORE = WOCO_DigitalIOState::CreateReadReply (pWOCO_DigitalIOState->get_IONumber (), ioState);
        }
        messageResult = UCOP::EMessageResult::SUCCESS;
      }
      else
      {
        messageResult = UCOP::EMessageResult::FAIL_CommandExecutionFailed;
      }
    }
    break;

  default:
    messageResult = UCOP::EMessageResult::FAIL_CommandNotSupported;
    break;
  }

  DeleteObject (m_pWOCO);

  m_ReplyData = UCOPData::CreateReplyData (m_RequestData, GetTimestamp (), messageResult);
  m_ReplyData.SetPayloadInfo (m_pPayloadSendBuffer, m_PayloadBuffersSize);

  if (pWORE != nullptr)
  {
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
