/**
 * @file clock_functions.h
 * @author Tina, Ørjan, Lasse og Adam
 */

#ifndef CLOCK_FUNCTIONS_H
#define CLOCK_FUNCTIONS_H

#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "mbed.h"
#include "structs.h"
#include <utility>

void startUp(DFRobot_RGBLCD1602 &lcd, std::string &longit, std::string &latit);
void tempHum(screen *scr);
void alarmFunc(alarmScreen_struct *info);
void defaultScreen(defaultScreen_struct *info);
void weatherFetch(weatherAuto_struct *info);
void weatherbyChoice(weatherChoice_struct *info);
void newsFetch(newsFetch_struct *info);

#endif // CLOCK_FUNCTIONS_H