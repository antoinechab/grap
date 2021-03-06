#include "components.h"

uint8_t slots[6] = {T_COMP_NULL};

/* To be changed, because for testing we assume we only have 1 value per component */
uint16_t values[6] = {0};
uint16_t alt_values[6] = {0}; /* Potentiometer */

void componentPlug(uint8_t type, uint8_t slot) {
  slots[slot] = type;

  switch(type) {
    case T_COMP_NULL:
      break;

    case T_COMP_PUSHBTN:
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][2]), INPUT);
      break;

    case T_COMP_DSWITCH:
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][0]), INPUT);
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][2]), INPUT);
      break;

    case T_COMP_POTENTIOMETER:
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][0]), OUTPUT);
      digitalWrite(pgm_read_byte(&SLOTS_PINS[slot][0]), LOW); // Ground
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][2]), INPUT);
      values[slot] = 1;
      alt_values[slot] = analogRead(pgm_read_byte(&SLOTS_PINS[slot][2]));
      break;

    case T_COMP_JOYSTICK:
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][0]), INPUT);
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][1]), INPUT);
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][2]), INPUT);
      break;

    case T_COMP_PIR:
      pinMode(pgm_read_byte(&SLOTS_PINS[slot][0]), INPUT);
      break;
  }
}

bool componentPollValues(uint8_t slot) {
  bool hasChanged = false;
  uint16_t old;
  switch(slots[slot]) {
    case T_COMP_NULL: ;
      break;

    case T_COMP_PUSHBTN: 
    {
      old = values[slot];
      values[slot] = digitalRead(pgm_read_byte(&SLOTS_PINS[slot][0]));
      if(values[slot] != old && values[slot] != 0) { hasChanged = true; }
      break;
    }

    case T_COMP_DSWITCH:
    {
      old = values[slot];
      bool switch1 = digitalRead(pgm_read_byte(&SLOTS_PINS[slot][0]));
      bool switch2 = digitalRead(pgm_read_byte(&SLOTS_PINS[slot][2]));

      if(!switch1 && !switch2) { values[slot] = 0; }
      if(!switch1 && switch2){ values[slot] = 1; } 
      if(switch1 && !switch2) { values[slot] = 2; } 
      if(switch1 && switch2) { values[slot] = 3; }
      
      if(values[slot] != old) { hasChanged = true; }
      break;
    }

    case T_COMP_POTENTIOMETER:
    {
      uint16_t oldRead = alt_values[slot];
      
      old = alt_values[slot];
      //Serial.println(map(analogRead(pgm_read_byte(&SLOTS_PINS[slot][2])), 0, 1023, 0, 255));
      // Avoid too frequent updates using an update threshold
      if(abs(map(analogRead(pgm_read_byte(&SLOTS_PINS[slot][2])), 0, 1023, 0, 255) - old) > POTENTIOMETER_THRESHOLD) {
        alt_values[slot] = map(analogRead(pgm_read_byte(&SLOTS_PINS[slot][2])), 0, 1023, 0, 255);

        if(map(analogRead(pgm_read_byte(&SLOTS_PINS[slot][2])), 0, 1023, 0, 255) < oldRead ) {
          values[slot] = 1;
        } else {
          values[slot] = 0;
        }
        
        hasChanged = true;
      }
      break;
    }

    case T_COMP_JOYSTICK:
    {
      int16_t cur_Y = analogRead(pgm_read_byte(&SLOTS_PINS[slot][1])) - JOYSTICK_CENTER;
      int16_t cur_X = analogRead(pgm_read_byte(&SLOTS_PINS[slot][2])) - JOYSTICK_CENTER;

      if(cur_X > JOYSTICK_THRESHOLD) {
        values[slot] = 0;
        hasChanged = true;
      } else if(cur_X < -JOYSTICK_THRESHOLD) {
        values[slot] = 1;
        hasChanged = true;
      } else if(cur_Y > JOYSTICK_THRESHOLD) {
        values[slot] = 3;
        hasChanged = true;
        //delay(1030 - map(abs(cur_Y), 0, JOYSTICK_CENTER, 30, 1000));
      } else if(cur_Y < -JOYSTICK_THRESHOLD) {
        values[slot] = 2;
        hasChanged = true;
      }

      break;
    }

    case T_COMP_PIR:
    {
      old = values[slot];
      values[slot] = digitalRead(pgm_read_byte(&SLOTS_PINS[slot][0]));
      if(values[slot] != old && values[slot] == 1) { hasChanged = true; }
      break;
    }


    default:
      break;
  }

  return hasChanged;
}

uint16_t componentLastValue(uint8_t slot) {
  return values[slot];
}
