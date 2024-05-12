#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "mbed.h"
#include <cstdio>
#include <string>

#include "certificates_hosts.h"
#include "helper_functions.h"
#include "json.hpp"

#define JSON_NOEXCEPTION
#define WAIT_TIME_MS (10000ms)
#define HTTP_RESPONSE_BUFFER_SIZE (4000)

using json = nlohmann::json;

void startUp(DFRobot_RGBLCD1602 &lcd) {

  time_t time;
  std::string longit;
  std::string latit;
  std::string city;

  constexpr int BUFFER_SIZE = 200;
  char *json_response = getInformation_Network(BUFFER_SIZE, ipgeolocationHost,
                                       ipgeolocationcert, ipgeoResource);

  // Parse response as JSON, starting from the first {
  json document = json::parse(json_response);

  if (document.is_discarded()) {
    printf("The input is invalid JSON\n");
    return;
  }

  // Get info from API
  // printf("If you came this far, then yay!\n");
  time = document["date_time_unix"].get<float>() +
         (document["timezone_offset_with_dst"].get<float>() * 3600);
  set_time(time);
  latit = document["geo"]["latitude"];
  longit = document["geo"]["longitude"];
  city = document["geo"]["city"];

  /*
    printf("Time as a basic string %s\n", ctime(&time));
    printf("Longitude is  %s\n", longit.c_str());
    printf("Latitude is %s\n", latit.c_str());
    printf("City is %s\n", city.c_str());
    */

  printf("Start up screens should be showing...\n");
  printf("Start screen 1\n");

  // Start up screen one
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Unix epoch time:");
  lcd.setCursor(0, 1);
  lcd.printf("%.0f", (float)time);
  ThisThread::sleep_for(2s);

  printf("Start screen 2\n");
  // Start up screen two
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Lat: %s", latit.c_str());
  lcd.setCursor(0, 1);
  lcd.printf("Lon: %s", longit.c_str());
  ThisThread::sleep_for(2s);

  printf("Start screen 3\n");
  // Start up screen three
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("City:");
  lcd.setCursor(0, 1);
  lcd.printf("%s", city.c_str());
  ThisThread::sleep_for(2s);

  lcd.clear();
};