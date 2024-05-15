/**
 * @file classes.h
 * @author Tina, Ørjan, Lasse og Adam
 */

#ifndef CLASSES_H
#define CLASSES_H

#include "mbed.h"

/**
 * @class Screen class is used as an intermediary between threads that produce
 * information and LCD screen (where information) is printed
 */

class screen {
private:
  char lineOne[20];
  char lineTwo[20];

public:
  Mutex messMut;

  void setLine_one(char *a) { strcpy(lineOne, a); }

  void setLine_two(char *a) { strcpy(lineTwo, a); }

  char *getLine_one() { return lineOne; }

  char *getLine_Two() { return lineTwo; }
};

#endif // CLASSES_H
