/**
 * @file structs.h
 * @author Tina, Ørjan, Lasse og Adam
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#include "classes.h"
#include "mbed.h"

typedef struct defaultScreen_struct {
  screen *defaultS;
  bool *alarmEn;
  bool *alarmAct;
  bool *alarmSn;
  bool *alarmMut;
  char *alarmBuf;
} defaultScreen_struct;

typedef struct alarmScreen_struct {
  screen *alarmS;
  int *screenN;
  bool *alarmEn;
  bool *alarmAct;
  bool *alarmSn;
  bool *alarmMut;
  bool *alarmChng;
  char *alarmBuf;
} alarmScreen_struct;

typedef struct weatherAuto_struct {
  screen *weatherS;
  int *screenN;
  Mutex *netMut;
  std::string *latit;
  std::string *longit;
} weatherAuto_struct;

typedef struct weatherChoice_struct {
  screen *weatherC;
  int *screenN;
  Mutex *netMut;
} weatherChoice_struct;

typedef struct newsFetch_struct {
  screen *newsS;
  int *screenN;
  Mutex *netMut;
  bool *ready;
  bool *loading;
} newsFetch_struct;

#endif // STRUCTS_H