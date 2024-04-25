#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"

void lcd_initialise(DFRobot_RGBLCD1602 &a);
void screenCheck(bool &screenChn, DFRobot_RGBLCD1602 &a, int &screenN);

#endif // HELPER_FUNCTIONS_H