#include <Streaming.h>

#include <STUC.h>

const bool     c_WriteConfig = false;  // <--- Set this flag to TRUE if the config has to be changed.
const uint16_t c_EepromOffset = 0;

const bool              m_DeviceIdsUsed = true;
const bool              m_MessageIdUsed = true;
const bool              m_TimestampUsed = false;
const uint32_t          m_DeviceId      = 0x63691401;
const EStucChecksumType m_ChecksumType  = EStucChecksumType::CRC16;

void setup ()
{
  Serial.begin (9600);
  delay (2000);

  EResult result;
  STUC stuc = STUC (c_EepromOffset, result);
  Serial << "STUC ctor, EEPROM READ, Result: " << (int)result << " = " << STUC::GetResultText (result) << endl;
  stuc.PrintConfig ();

  if (!c_WriteConfig)
    return;

  stuc = STUC (m_DeviceIdsUsed,
               m_MessageIdUsed,
               m_TimestampUsed,
               m_DeviceId,
               m_ChecksumType,
               result);
  Serial << "STUC ctor, new config, Result: " << (int)result << " = " << STUC::GetResultText (result) << endl;
  stuc.PrintConfig ();

  result = stuc.WriteConfigToEEPROM (c_EepromOffset);
  Serial << "STUC WriteConfigToEEPROM, EEPROM WRITE, Result: " << (int)result << " = " << STUC::GetResultText (result) << endl;

  stuc = STUC (c_EepromOffset, result);
  Serial << "STUC ctor, EEPROM READ, Result: " << (int)result << " = " << STUC::GetResultText (result) << endl;
  stuc.PrintConfig ();
}

void loop ()
{
}
