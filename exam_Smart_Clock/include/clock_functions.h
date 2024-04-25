#ifndef CLOCK_FUNCTIONS_H
#define CLOCK_FUNCTIONS_H

#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "structs.h"
#include <utility>


void startUp(DFRobot_RGBLCD1602 &lcd);
void tempHum(screen* scr);
void alarmFunc(std::pair<screen*, bool*> *info);
void defaultScreen(std::pair<screen*, bool*> *info);

#endif // CLOCK_FUNCTIONS_H