#ifndef TOBA_h
#define TOBA_h

#include <Result.h>

//--------------------------------------------------------------------
// TOBA: TObi's Building Automation
//--------------------------------------------------------------------
class TOBA
{
//==================== Enums ====================
public:
  enum class EResult : uint16_t
  {
    Dummy_FirstClassFailure = (uint16_t)::EResult::Dummy_FirstClassFailure,
    #define X(name) name,
    #include "TOBA_EResult_failures.h"
    #undef X
    Dummy_LastClassFailure
  };

//==================== Fields ====================
private:
  //-------------------- static --------------------

  static const char* const c_EResult_ClassFailures_Names[] PROGMEM;

  #define X(name) static const char _EResult_##name[] PROGMEM;
  #include "TOBA_EResult_failures.h"
  #undef X

//==================== Constructors ====================
public:
  TOBA () = delete;

//==================== Public Methods ====================
public:
  //-------------------- static --------------------

  static const __FlashStringHelper* GetResultText (::EResult i_Result);
};

#endif
