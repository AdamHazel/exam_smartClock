#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"

void lcd_initialise(DFRobot_RGBLCD1602 &a);
void screenCheck(bool &screenChn, DFRobot_RGBLCD1602 &a, int &screenN);
char* getInformation_Network(int BUFFER_SIZE_REQUEST,  const char* hostChoice, const char* certificate, const char* resourceWanted);

#endif // HELPER_FUNCTIONS_H