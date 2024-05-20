/**
 * @file main.cpp
 * @author Tina, Ørjan, Lasse og Adam
 */

#include "DFRobot_RGBLCD1602.h"
#include "HTS221Sensor.h"
#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <string.h>
#include <string>
#include <utility>

#include "classes.h"
#include "clock_functions.h"
#include "helper_functions.h"
#include "structs.h"

/**
 * @brief Global variables used for changing the screen on clock
 */
const int screenAmount = 6;
int buttonClick = 0;
int screenNumber = 0;
bool screenChanged = false;

/**
 * @brief Global variables used for controlling alarm. These are used by alarm
 * and default screen thread
 */
bool alarmChange = false;
bool alarmActive = false;
bool alarmEnabled = false;
bool alarmSnoozed = false;
bool alarmMuted = false;
char alarmBuffer[17];

/**
 * @brief Global variables used for news screen
 */
bool newsloading = true;
bool readytoSend = false;

/**
 * @brief Global variables used for accessing weather data
 */
std::string latit;
std::string longit;

/**
 * @brief Class initialisation for screen
 */
I2C lcdI2C(PB_9, PB_8);
DFRobot_RGBLCD1602 lcd(&lcdI2C);

/**
 * @brief Mutex to ensure that functions in threads cannot access network
 * interface at the same time
 * @see src/weatherbyChoice.cpp
 * @see src/weatherFetch.cpp
 * @todo reference news function
 */
Mutex networkMutex;

BufferedSerial serial_port(USBTX, USBRX);

/**
 * @brief Buttons used to control clock functionality
 */
InterruptIn BlueButton(BUTTON1, PullNone);
InterruptIn enableAlarm(PA_3, PullUp);
InterruptIn snoozeAlarm(PB_4, PullUp);
InterruptIn muteAlarm(PA_15, PullUp);
InterruptIn setAlarm(PB_2, PullUp);

/**
 * @brief Calculate screen number and report that screen has changed
 */
void screenChange() {
  ++buttonClick;
  screenNumber = buttonClick % screenAmount;
  screenChanged = true;
}

/**
 * @brief Switches alarm state between enabled and disabled
 */
void enableAlarm_func() {
  if (alarmEnabled == false) {
    alarmEnabled = true;
  } else if (alarmEnabled == true && alarmActive == false &&
             alarmSnoozed == false) {
    alarmEnabled = false;
    alarmMuted = false;
    alarmActive = false;
    alarmSnoozed = false;
  }
}

/**
 * @brief Snoozes an active (ringing) alarm
 */
void snoozeAlarm_func() {
  if (alarmEnabled == true && alarmActive == true && alarmSnoozed == false) {
    alarmSnoozed = true;
    alarmActive = false;
  }
}

/**
 * @brief Mutes an active (ringing) alarm
 */
void muteAlarm_func() {
  if ((alarmActive == true || alarmSnoozed == true) && alarmMuted == false)
    alarmMuted = true;
}

/**
 * @brief Ensures that it is safe to increase hour or minute counter when
 * setting alarm
 * @see src/alarmFunction.cpp
 */
void setAlarm_func() {
  if (screenNumber == 1 && alarmChange == false && alarmEnabled == false) {
    alarmChange = true;
  }
}

int main() {

  serial_port.set_baud(115200);

  /**
   * @brief Starts up LCD screen
   * @see src/helper_functions.cpp
   */
  lcd_initialise(lcd);

  /**
   * @brief Starts up clock
   * @see src/startUp.cpp
   */
  startUp(lcd, longit, latit);

  /**
   * @brief Screen objects are used so that threads can share data to be shown
   * on screen
   * @see include/classes.h
   */
  screen *defScreen = new screen;
  screen *alarmScreen = new screen;
  screen *tempScreen = new screen;
  screen *weathScreen = new screen;
  screen *weathChoiceScreen = new screen;
  screen *newsScreen = new screen;

  /**
   * @brief Assign pointers to needed information for each struct used by
   * threads
   * @see include/structs.h
   */
  defaultScreen_struct *defThreadInfo = new defaultScreen_struct;
  defThreadInfo->defaultS = defScreen;
  defThreadInfo->alarmAct = &alarmActive;
  defThreadInfo->alarmEn = &alarmEnabled;
  defThreadInfo->alarmMut = &alarmMuted;
  defThreadInfo->alarmSn = &alarmSnoozed;
  defThreadInfo->alarmBuf = alarmBuffer;

  alarmScreen_struct *alarmThreadInfo = new alarmScreen_struct;
  alarmThreadInfo->alarmS = alarmScreen;
  alarmThreadInfo->screenN = &screenNumber;
  alarmThreadInfo->alarmAct = &alarmActive;
  alarmThreadInfo->alarmEn = &alarmEnabled;
  alarmThreadInfo->alarmMut = &alarmMuted;
  alarmThreadInfo->alarmChng = &alarmChange;
  alarmThreadInfo->alarmSn = &alarmSnoozed;
  alarmThreadInfo->alarmBuf = alarmBuffer;

  weatherAuto_struct *weatherThreadInfo = new weatherAuto_struct;
  weatherThreadInfo->weatherS = weathScreen;
  weatherThreadInfo->screenN = &screenNumber;
  weatherThreadInfo->netMut = &networkMutex;
  weatherThreadInfo->latit = &latit;
  weatherThreadInfo->longit = &longit;

  weatherChoice_struct *choiceThreadInfo = new weatherChoice_struct;
  choiceThreadInfo->weatherC = weathChoiceScreen;
  choiceThreadInfo->screenN = &screenNumber;
  choiceThreadInfo->netMut = &networkMutex;

  newsFetch_struct *newsScreenInfo = new newsFetch_struct;
  newsScreenInfo->newsS = newsScreen;
  newsScreenInfo->screenN = &screenNumber;
  newsScreenInfo->netMut = &networkMutex;
  newsScreenInfo->ready = &readytoSend;
  newsScreenInfo->loading = &newsloading;

  // Attaching functions to interrupts
  BlueButton.rise(&screenChange);
  enableAlarm.fall(&enableAlarm_func);
  snoozeAlarm.fall(&snoozeAlarm_func);
  muteAlarm.fall(&muteAlarm_func);
  setAlarm.fall(&setAlarm_func);

  // Starting threads
  Thread tempInfo(osPriorityNormal, OS_STACK_SIZE, nullptr, "tempScreen");
  Thread defaultInfo(osPriorityNormal, OS_STACK_SIZE, nullptr, "defScreen");
  Thread alarmSet(osPriorityNormal, OS_STACK_SIZE, nullptr, "alarmSetScreen");
  Thread weatherAuto(osPriorityNormal, OS_STACK_SIZE, nullptr, "weatherScreen");
  Thread weatherChoice(osPriorityNormal, OS_STACK_SIZE, nullptr,
                       "weatherChoiceScreen");
  Thread newsFetching(osPriorityNormal, OS_STACK_SIZE, nullptr, "newsScreen");

  tempInfo.start(callback(tempHum, tempScreen));
  defaultInfo.start(callback(defaultScreen, defThreadInfo));
  alarmSet.start(callback(alarmFunc, alarmThreadInfo));
  weatherAuto.start(callback(weatherFetch, weatherThreadInfo));
  weatherChoice.start(callback(weatherbyChoice, choiceThreadInfo));
  newsFetching.start(callback(newsFetch, newsScreenInfo));

  /**
   * @brief Prints corresponding information on chosen screen
   */
  while (true) {
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
      weathScreen->messMut.lock();
      lcd.printf(weathScreen->getLine_one());
      lcd.setCursor(0, 1);
      lcd.printf(weathScreen->getLine_Two());
      weathScreen->messMut.unlock();
      break;
    case 4: // Weather by choice screen
      screenCheck(screenChanged, lcd, screenNumber);
      lcd.setCursor(0, 0);
      weathChoiceScreen->messMut.lock();
      lcd.printf(weathChoiceScreen->getLine_one());
      lcd.setCursor(0, 1);
      lcd.printf(weathChoiceScreen->getLine_Two());
      weathChoiceScreen->messMut.unlock();
      break;
    case 5: // News screen
      screenCheck(screenChanged, lcd, screenNumber);
      if (newsloading == true) {
        lcd.setCursor(0, 0);
        newsScreen->messMut.lock();
        lcd.printf(newsScreen->getLine_one());
        lcd.setCursor(0, 1);
        lcd.printf(newsScreen->getLine_Two());
        newsScreen->messMut.unlock();
      }
      if (readytoSend == true) {
        lcd.setCursor(0, 0);
        newsScreen->messMut.lock();
        lcd.printf(newsScreen->getLine_one());
        lcd.setCursor(0, 1);
        lcd.printf(newsScreen->getLine_Two());
        newsScreen->messMut.unlock();
        readytoSend = false;
      }
      break;
    }
  }

  // End threads
  tempInfo.join();
  defaultInfo.join();
  alarmSet.join();
  weatherAuto.join();
  weatherChoice.join();
  newsFetching.join();

  // End of structs
  delete defThreadInfo;
  delete alarmThreadInfo;
  delete weatherThreadInfo;
  delete choiceThreadInfo;
  delete newsScreenInfo;

  // End of screens
  delete defScreen;
  delete alarmScreen;
  delete tempScreen;
  delete weathScreen;
  delete weathChoiceScreen;
  delete newsScreen;

  return 0;
}
