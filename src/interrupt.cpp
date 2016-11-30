#include <Arduino.h>
#include "interrupt.h"


static void (*callback)(int pin);


void intr0(void)  { callback(0);  }
void intr1(void)  { callback(1);  }
void intr2(void)  { callback(2);  }
void intr3(void)  { callback(3);  }
void intr4(void)  { callback(4);  }
void intr5(void)  { callback(5);  }
void intr12(void) { callback(12); }
void intr13(void) { callback(13); }
void intr14(void) { callback(14); }
void intr15(void) { callback(15); }


void accept_interrupt(int pin, void (*cb)(int)) {
    void (*intr)(void);

    switch (pin) {
    case 0:  intr = intr0;  break;
    case 1:  intr = intr1;  break;
    case 2:  intr = intr2;  break;
    case 3:  intr = intr3;  break;
    case 4:  intr = intr4;  break;
    case 5:  intr = intr5;  break;
    case 12: intr = intr12; break;
    case 13: intr = intr13; break;
    case 14: intr = intr14; break;
    case 15: intr = intr15; break;
    default:
        Serial.println("firmware: unsupported interrupt pin");
        return;
    }

    callback = cb;
    attachInterrupt(pin, intr, CHANGE);
}
