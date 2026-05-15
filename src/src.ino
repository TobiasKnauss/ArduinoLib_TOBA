#include <Arduino.h>
#include <Streaming.h>

#include "TOBA_Worker.h"

TOBA_Worker* m_pTobaWorker = 0;


//--------------------------------------------------------------------
void setup ()
{
  EResult result;

  Serial.begin (9600);
  Serial1.begin (9600);
  delay (2000);

  m_pTobaWorker = new TOBA_Worker (&Serial1, 80, 50, 20, result);
  
}

//--------------------------------------------------------------------
void loop ()
{
  EResult result;




    Serial.println ("Idle.");
    delay (1000);
}
