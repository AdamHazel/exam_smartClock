/*
*/

#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"
#include "HTS221Sensor.h"
#include <cstdio>
#include <cstring>
#include <string.h>
#include <utility>

#include "classes.h"
#include "helper_functions.h"
#include "clock_functions.h"
#include "structs.h"

// Constants and screen variables
const int screenAmount = 6;
int buttonClick = 0;
int screenNumber = 0;
bool screenChanged = false;
bool alarmActive = false;

// Buttons
InterruptIn BlueButton(BUTTON1, PullNone);

// I2C class for screen
I2C lcdI2C(PB_9, PB_8);
DFRobot_RGBLCD1602 lcd(&lcdI2C);

// Buffered serial class
BufferedSerial serial_port(USBTX, USBRX);

// Mutexs
Mutex networkMutex;

// Interrupt functions
void screenChange() {
    ++buttonClick;
    screenNumber = buttonClick % screenAmount; 
    screenChanged = true;
}




int main() {
    
    serial_port.set_baud(115200);
    
    //Initialise screen
    lcd_initialise(lcd);
    startUp(lcd);

    // Objects for printing information on screen
    screen *defScreen = new screen;
    screen *alarmScreen = new screen;
    screen *tempScreen = new screen;
    screen *weathScreen = new screen;
    screen *coorScreen = new screen;
    screen *newsScreen = new screen;

    // Pairs for sending into threads
    std::pair<screen*, bool*> *defPair = new std::pair<screen*, bool*>;
    defPair->first = defScreen;
    defPair->second = &alarmActive;


    // USE A STRUCT FOR THIS INSTEAD AS WE NEED SCREEN NUMBER
    std::pair<screen*, bool*> *alarmPair = new std::pair<screen*, bool*>;
    alarmPair->first = alarmScreen;
    alarmPair->second = &alarmActive;

    
    
    // Struct for sending in all necessary information into alarm thread
    alarmScreen_struct* alarm_T_info = new alarmScreen_struct;
    alarm_T_info->alarmS_pointer = alarmScreen;
    alarm_T_info->screenN = &screenNumber;
    alarm_T_info->alarmA_boolPointer = &alarmActive;
    

    printf("We got passed assigning to struct\n");


    // Interrupt for changing screen
    BlueButton.rise(&screenChange);

    // Threads
    Thread tempInfo(osPriorityNormal, OS_STACK_SIZE, nullptr, "tempScreen");
    Thread defaultInfo(osPriorityNormal, OS_STACK_SIZE, nullptr, "defScreen");
    Thread alarmSet(osPriorityNormal, OS_STACK_SIZE, nullptr, "alarmSetScreen");

    printf("Initialise threads\n");

    // Start thread
    tempInfo.start(callback(tempHum, tempScreen));
    defaultInfo.start(callback(defaultScreen, defPair));
    alarmSet.start(callback(alarmFunc,alarmPair));

    printf("Threads started\n");
    

    while(true) 
    {
        switch (screenNumber) {
        case 0: // Default screen
            screenCheck(screenChanged, lcd, screenNumber);
            lcd.setCursor(0, 0);
            defScreen->messMut.lock();
            lcd.printf(defScreen->getLine_one());
            lcd.setCursor(0, 1);
            lcd.printf(defScreen->getLine_Two());
            defScreen->messMut.unlock();
            break;
        case 1:
            screenCheck(screenChanged, lcd, screenNumber);
            lcd.setCursor(0, 0);
            alarmScreen->messMut.lock();
            lcd.printf(alarmScreen->getLine_one());
            lcd.setCursor(0, 1);
            lcd.printf(alarmScreen->getLine_Two());
            alarmScreen->messMut.unlock();
            break;
        case 2: // Temperature and screen
            screenCheck(screenChanged, lcd, screenNumber);
            lcd.setCursor(0, 0);
            tempScreen->messMut.lock();
            lcd.printf(tempScreen->getLine_one());
            lcd.setCursor(0, 1);
            lcd.printf(tempScreen->getLine_Two());
            lcd.printf("%%");
            tempScreen->messMut.unlock();
            break;
        case 3: // Weather screen
            screenCheck(screenChanged, lcd, screenNumber);
            lcd.setCursor(0, 0);
            lcd.printf("Weather screen");
            break;
        case 4: // Coordinates screen
            screenCheck(screenChanged, lcd, screenNumber);
            lcd.setCursor(0, 0);
            lcd.printf("Coor. screen");
            break;
        case 5: // News screen
            screenCheck(screenChanged, lcd, screenNumber);
            lcd.setCursor(0, 0);
            lcd.printf("News screen");
            break;
        }
    }

 
    // End threads
    tempInfo.join();
    defaultInfo.join();
    alarmSet.join();

    return 0;
}


