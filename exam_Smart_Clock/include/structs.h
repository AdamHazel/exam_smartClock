#ifndef STRUCTS_H
#define STRUCTS_H

#include "classes.h"
#include "mbed.h"

typedef struct alarmScreen_struct {
  screen *alarmS_pointer;
  int* screenN;
  bool *alarmA_boolPointer;

} alarmScreen_struct;

#endif // STRUCTS_H