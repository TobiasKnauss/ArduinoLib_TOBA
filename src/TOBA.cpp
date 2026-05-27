#include "TOBA.h"

//--------------------------------------------------------------------
#define X(name) const char TOBA::_EResult_##name[] PROGMEM = #name;
#include "TOBA_EResult_failures.h"
#undef X

//--------------------------------------------------------------------
#define X(name) _EResult_##name,
const char* const TOBA::c_EResult_ClassFailures_Names[] PROGMEM =
{
  #include "TOBA_EResult_failures.h"
};
#undef X

//--------------------------------------------------------------------
const __FlashStringHelper* TOBA::GetResultText (::EResult i_Result)
{
  if ((uint16_t)i_Result < (uint16_t)EResult::Dummy_FirstClassFailure)
    return Result::GetText (i_Result);
  return (const __FlashStringHelper*)pgm_read_ptr(&c_EResult_ClassFailures_Names[(uint16_t)i_Result - (uint16_t)EResult::Dummy_FirstClassFailure - 1]);
}
