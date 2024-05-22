/**
 * @file alarmFunction.cpp
 * @author Tina, Ørjan, Lasse og Adam
 */

#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "mbed.h"
#include "structs.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <utility>

#define BUFFER_SIZE 17

using namespace std::chrono;

/**
 * @brief Function to control alarm.
 * @param alarmScreen_struct pointer
 * @see structs.h
 */

void alarmFunc(alarmScreen_struct *info) {
  // IO devices
  DigitalIn timePick(PA_0, PullDown);
  PwmOut buzzer(PA_7);
  buzzer.period(1.0 / 2000);

  // Char arrays to place information that will be displayed on screen
  static char buffer[BUFFER_SIZE];
  static char buffer2[BUFFER_SIZE];

  // Constant expressions connected to setting alarm and duration of alarm ring
  // and snooze
  static constexpr int hourInterval{24};
  static constexpr int minInterval{60};
  static constexpr int alarmDurS{60 * 10};
  static constexpr int snoozeDurS{60 * 5};

  // Variables to allow user to set alarm
  static int hourCounter{0};
  static int hourMod{24};
  static int minCounter{0};
  static int minMod{60};

  // Variables to hold chosen alarm time
  static int hour{0};
  static int min{0};

  // Timers and variable for alarm and snooze functionality
  static Timer alarmT;
  static Timer snoozeT;
  static int secondCounter{0};

  snprintf(buffer, BUFFER_SIZE, "Alarm:");

  while (true) {
    // State 0
    if (*(info->alarmAct) == false && *(info->alarmEn) == false &&
        *(info->alarmSn) == false && *(info->alarmMut) == false) {
      alarmT.stop();
      alarmT.reset();

      snoozeT.stop();
      snoozeT.reset();

      buzzer.write(0.0);
    }

    // State 1 : To set alarm clock
    if (*(info->screenN) == 1 && *(info->alarmAct) == false &&
        *(info->alarmEn) == false && *(info->alarmSn) == false &&
        *(info->alarmMut) == false) {
      if (timePick.read() == 0 && *(info->alarmChng) == true) {
        ++hourCounter;
        hour = hourCounter % hourMod;
        *(info->alarmChng) = false;
      }
      if (timePick.read() == 1 && *(info->alarmChng) == true) {
        ++minCounter;
        min = minCounter % minMod;
        *(info->alarmChng) = false;
      }
      snprintf(buffer2, BUFFER_SIZE, "%02d:%02d", hour, min);
    }

    // State 2 : Alarm enabled
    if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
        *(info->alarmSn) == false) {
      // Stop and reset timers and buzzer
      alarmT.stop();
      alarmT.reset();

      snoozeT.stop();
      snoozeT.reset();

      buzzer.write(0.0);

      // Record alarm time
      snprintf(buffer2, BUFFER_SIZE, "%02d:%02d", hour, min);
      snprintf(info->alarmBuf, BUFFER_SIZE, "%s", buffer2);

      // Create string to record recurring alarm timestamp
      char alarmCheck[BUFFER_SIZE];
      time_t seconds = time(NULL);
      strftime(alarmCheck, BUFFER_SIZE, "%R", localtime(&seconds));

      // Handling of a muted alarm whilst current time and alarm timestamp is
      // still equal
      if (*(info->alarmMut) == false) {
        if (strcmp(alarmCheck, info->alarmBuf) == 0) {
          *(info->alarmAct) = true;
        }
      } else if (*(info->alarmMut) == true) {
        if (strcmp(alarmCheck, info->alarmBuf) != 0) {
          *(info->alarmMut) = false;
        }
      }
    }

    // State 3 : Alarm active
    if (*(info->alarmAct) == true && *(info->alarmEn) == true &&
        *(info->alarmSn) == false && *(info->alarmMut) == false) {
      // Alarm sounds
      alarmT.start();
      buzzer.write(0.2);
      secondCounter =
          (int)duration_cast<seconds>(alarmT.elapsed_time()).count();

      // Alarm no longer active after a given time
      if (secondCounter == alarmDurS) {
        buzzer.write(0.0);
        alarmT.stop();
        alarmT.reset();
        *(info->alarmAct) = false;
      }
    }

    // State 4 : Alarm snoozed
    if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
        *(info->alarmSn) == true && *(info->alarmMut) == false) {
      buzzer.write(0.0);
      alarmT.stop();
      alarmT.reset();

      snoozeT.start();
      secondCounter =
          (int)duration_cast<seconds>(snoozeT.elapsed_time()).count();

      // Snooze timer
      if (secondCounter == snoozeDurS) {
        snoozeT.stop();
        snoozeT.reset();
        *(info->alarmAct) = true;
        *(info->alarmSn) = false;
      }
    }

    // State 5 : Alarm muted
    if ((*(info->alarmAct) == true || *(info->alarmSn) == true) &&
        *(info->alarmEn) == true && *(info->alarmMut) == true) {
      *(info->alarmSn) = false;
      *(info->alarmAct) = false;
    }

    // Send display information to shared buffer
    info->alarmS->messMut.lock();
    info->alarmS->setLine_one(buffer);
    info->alarmS->setLine_two(buffer2);
    info->alarmS->messMut.unlock();
  }
};
