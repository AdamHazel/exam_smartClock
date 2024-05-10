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

// Alarm states and buffer
bool alarmChange = false;
bool alarmActive = false;
bool alarmEnabled = false;
bool alarmSnoozed = false;
bool alarmMuted = false;
char alarmBuffer[17];

// Buttons (Port 0 and 1 is used in alarmFunction)
InterruptIn BlueButton(BUTTON1, PullNone);
InterruptIn enableAlarm(PA_3, PullUp);
InterruptIn snoozeAlarm(PB_4, PullUp);
InterruptIn muteAlarm(PA_15, PullUp);
InterruptIn setAlarm(PB_2, PullUp);

// Interrupt functions
void screenChange() {
    ++buttonClick;
    screenNumber = buttonClick % screenAmount; 
    screenChanged = true;
}

void enableAlarm_func() {
    if (alarmEnabled == false) {
        alarmEnabled = true;
    } else if (alarmEnabled == true && alarmActive == false && alarmSnoozed == false) {
        alarmEnabled = false;
        alarmMuted = false;
        alarmActive = false;
        alarmSnoozed = false;
    }
}

// Cannot snooze unless alarm is active
void snoozeAlarm_func() {
    if (alarmEnabled == true && alarmActive == true && alarmSnoozed == false) {
        alarmSnooaed = true;
        alarmActive = false;
    }
}

// Cannot mute unless alarm is active or snoozed
void muteAlarm_func() {
    if ((alarmActive == true || alarmSnoozed == true) && alarmMuted == false)
        alarmMuted = true;
}

// To increase alarm values
void setAlarm_func() {
    if (screenNumber == 1 && alarmChange == false)
    {
        alarmChange = true;
    }
}

// I2C class for screen
I2C lcdI2C(PB_9, PB_8);
DFRobot_RGBLCD1602 lcd(&lcdI2C);

// Buffered serial class
BufferedSerial serial_port(USBTX, USBRX);

// Mutexs
Mutex networkMutex;


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

    // Initialise struct for default screen thread
    defaultScreen_struct* defThreadInfo = new defaultScreen_struct;
    defThreadInfo->defaultS = defScreen;
    defThreadInfo->alarmAct = &alarmActive;
    defThreadInfo->alarmEn = &alarmEnabled;
    defThreadInfo->alarmMut = &alarmMuted;
    defThreadInfo->alarmSn = &alarmSnoozed;
    defThreadInfo->alarmBuf = alarmBuffer;

    // Initialise strunct for alarm screeen thread
    alarmScreen_struct* alarmThreadInfo = new alarmScreen_struct;
    alarmThreadInfo->alarmS = alarmScreen;
    alarmThreadInfo->screenN = &screenNumber;
    alarmThreadInfo->alarmAct = &alarmActive;
    alarmThreadInfo->alarmEn = &alarmEnabled;
    alarmThreadInfo->alarmMut = &alarmMuted;
    alarmThreadInfo->alarmSn = &alarmSnoozed;
    alarmThreadInfo->alarmChng = &alarmChange;
    alarmThreadInfo->alarmBuf = alarmBuffer;
    

    printf("We got passed assigning to struct\n");

    // Interrupt for changing screen
    BlueButton.rise(&screenChange);
    enableAlarm.fall(&enableAlarm_func);
    snoozeAlarm.fall(&snoozeAlarm_func);
    muteAlarm.fall(&muteAlarm_func);
    setAlarm.fall(&setAlarm_func);

    // Threads
    Thread tempInfo(osPriorityNormal, OS_STACK_SIZE, nullptr, "tempScreen");
    Thread defaultInfo(osPriorityNormal, OS_STACK_SIZE, nullptr, "defScreen");
    Thread alarmSet(osPriorityNormal, OS_STACK_SIZE, nullptr, "alarmSetScreen");

    printf("Initialise threads\n");

    // Start thread
    tempInfo.start(callback(tempHum, tempScreen));
    defaultInfo.start(callback(defaultScreen, defThreadInfo));
    alarmSet.start(callback(alarmFunc,alarmThreadInfo));

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
        case 1: // Alarm screen
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


