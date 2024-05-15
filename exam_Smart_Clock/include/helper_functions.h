/**
 * @file helper_functions.h
 * @author Tina, Ørjan, Lasse og Adam
 */
 
#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "DFRobot_RGBLCD1602.h"
#include "mbed.h"


void lcd_initialise(DFRobot_RGBLCD1602 &a);
void screenCheck(bool &screenChn, DFRobot_RGBLCD1602 &a, int &screenN);
char *getInformation_https(int BUFFER_SIZE_REQUEST, const char *hostChoice,
                           const char *certificate, const char *resourceWanted);
char *getInformation_http(int BUFFER_SIZE_REQUEST, const char *hostChoice,
                          char *resourceWanted);

#endif // HELPER_FUNCTIONS_H