/*
 Angelean Clock
 Alex Beal 2012 <alexlbeal@gmail.com> http://usrsb.in
*/

#include <LiquidCrystal.h>
#include "Angelean.h"
#include "names.h"

// For buttons LOW == button ISDOWN
#define ISDOWN LOW
#define ISUP   HIGH

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int upPin = 8;
const int downPin = 9;
const int rightPin = 7;
const int modePin = 13;

Angelean aClock(000.000, 2012);

/**
 * Spin while 'pin' is high or low.
 * spinWhile(9, HIGH) // Spins while the pin is HIGH
 **/
void spinWhile(int pin, int highOrLow){
    while(digitalRead(pin) == highOrLow);
}

// Print the main clock display
void printClock(){
    char timeStr[17];
    char infoStr[17];
    char temp[17];
    AngeleanTime aTime;

    lcd.clear();
    // Get and print the time.
    aClock.getTimeStr(&(timeStr[0]));
    lcd.setCursor(0, 0);
    lcd.print(timeStr);

    // Get and print the info str (week, day, leap year)
    aClock.getTime(&aTime);

    infoStr[0]='\0';
    strcat(&(infoStr[0]), "Week ");
    itoa(aTime.week, &(temp[0]), 10);
    strcat(&(infoStr[0]), temp);

    strcat(&(infoStr[0]), " Day ");
    itoa(aTime.weekDay, &(temp[0]), 10);
    strcat(&(infoStr[0]), temp);

    if(aTime.leapYear){
        strcat(&(infoStr[0]), " LY ");
    }
    infoStr[16] = '\0';

    lcd.setCursor(0,1);
    lcd.print(infoStr);
}

ClockMode runTest(){
    AngeleanTime time;

    spinWhile(modePin, ISDOWN);
    while(true){
        aClock.getTime(&time);

        lcd.clear();
        lcd.print("Test mode:");
        lcd.setCursor(0,1);
        lcd.print(time.millisDiff/100UL);
        if(digitalRead(modePin) == LOW){
            return Clock;
        }
        delay(100);
    }
}


ClockMode runClock(){
    /*AngeleanTime aTime;*/
    // Bottom string containing week, day, leap year info.
    static unsigned long lastUpdate=millis();

    spinWhile(modePin, ISDOWN);
    while(true){
        // Update display every 1000 ms
        if(millis() - lastUpdate >= 1000UL){
            printClock();
            lastUpdate=millis();
        }

        // If modePin is held down for 1000 ms, switch to set clock mode.
        if (digitalRead(modePin) == ISDOWN){
            unsigned long startPush = millis();
            while(digitalRead(modePin) == ISDOWN){
                if(millis() - startPush >= 1000UL){
                    return Set;
                }
            }
            return Test;
        }
    }
}

ClockMode setClock(){
    char timeStr[16];
    char dispBuf[16];
    const char nextIntTable[] = {'1','2','3','4','5','6','7','8','9','0'};
    const char prevIntTable[] = {'9','0','1','2','3','4','5','6','7','8'};
    int selectedDigit = 0;
    unsigned long blinkTimer = millis();
    unsigned int blinkStart = 0;
    unsigned int blinkEnd = 0;
    bool blinked = false;

    lcd.clear();
    lcd.print("Set the clock:");
    aClock.getTimeStr(&(timeStr[0]));
    // Spin while the mode change button is held
    spinWhile(modePin, ISDOWN);
    while(true){
        bool upPressed = false, downPressed = false;
        if( (upPressed = (digitalRead(upPin) == ISDOWN)) || (downPressed = (digitalRead(downPin) == ISDOWN)) ){
            while(digitalRead(upPin) == ISDOWN || digitalRead(downPin) == ISDOWN);
            if(selectedDigit <= 6){
                if(upPressed){
                    timeStr[selectedDigit] = nextIntTable[timeStr[selectedDigit] - '0'];
                } else if(downPressed){
                    timeStr[selectedDigit] = prevIntTable[timeStr[selectedDigit] - '0'];
                }
            } else {
                int year = atoi(&(timeStr[9]));
                if(upPressed){
                    year++;
                } else if(downPressed){
                    year--;
                }
                itoa(year, &(timeStr[9]), 10);
            }
        } else if(digitalRead(rightPin) == ISDOWN){
            // Spin while the button is held.
            spinWhile(rightPin, ISDOWN);
            if(selectedDigit == 2){
                // Skip the decimal.
                selectedDigit += 2;
            }else if(selectedDigit == 6){
                // Skip the ", "
                selectedDigit += 3;
            }else if(selectedDigit == 9){
                // Wrap around.
                selectedDigit = 0;
            }else{
                selectedDigit +=1;
            }

            if(selectedDigit == 9){
                // Blink the entire year (digits 9-12)
                blinkStart = 9;
                blinkEnd = 12;
            } else {
                blinkStart = selectedDigit;
                blinkEnd = selectedDigit;
            }
        } else if(digitalRead(modePin) == LOW){
            aClock.setTimeStr(&(timeStr[0]));
            return Clock;
        }

        strncpy(dispBuf, timeStr, 16);
        // Blink every 500 millis
        if(millis() - blinkTimer >= 500UL){
            blinkTimer = millis();
            blinked = !blinked;
        }
        if(blinked){
            // Set the digits that are supposed to be blinking to ' '
            for(unsigned int i = blinkStart; i<=blinkEnd; i++){
                dispBuf[i] = ' ';
            }
        }
        lcd.setCursor(0,1);
        lcd.print(dispBuf);
    }
}

void setup() {
    pinMode(upPin, INPUT_PULLUP);
    pinMode(downPin, INPUT_PULLUP);
    pinMode(rightPin, INPUT_PULLUP);
    pinMode(modePin, INPUT_PULLUP);
    // set up the LCD's number of columns and rows: 
    lcd.begin(16, 2);
    Serial.begin(9600);
}

void loop() {
    ClockMode curMode = Clock;

    while(true){
        switch(curMode){
            case Clock:
                Serial.println("About to run clock.");
                curMode = runClock();
                break;
            case Set:
                curMode = setClock();
                break;
            case Test:
                curMode = runTest();
                break;
        }
    }
}

