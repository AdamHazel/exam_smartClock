#ifndef CLOCK_FUNCTIONS_H
#define CLOCK_FUNCTIONS_H

#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "structs.h"
#include <utility>


void startUp(DFRobot_RGBLCD1602 &lcd);
void tempHum(screen* scr);
void alarmFunc(alarmScreen_struct* info);
void defaultScreen(defaultScreen_struct* info);

#endif // CLOCK_FUNCTIONS_H