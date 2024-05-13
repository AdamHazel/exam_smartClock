#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "mbed.h"
#include <cstdio>
#include <string>

#include "certificates_hosts.h"
#include "helper_functions.h"
#include "json.hpp"

#define JSON_NOEXCEPTION


using json = nlohmann::json;

void startUp(DFRobot_RGBLCD1602 &lcd, std::string &longit, std::string &latit) {

  time_t time;
  std::string city;

  constexpr int BUFFER_SIZE = 200;
  char *json_response = getInformation_https(BUFFER_SIZE, ipgeolocationHost,
                                       ipgeolocationcert, ipgeoResource);

  
  printf("JSON RESPONSE AGAIN TO CHECK:\n\n%s\n\n", json_response);
  
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


  printf("Start up screens should be showing...\n");
  
  // Start up screen one
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Unix epoch time:");
  lcd.setCursor(0, 1);
  lcd.printf("%.0f", (float)time);
  ThisThread::sleep_for(2s);

  // Start up screen two
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Lat: %s", latit.c_str());
  lcd.setCursor(0, 1);
  lcd.printf("Lon: %s", longit.c_str());
  ThisThread::sleep_for(2s);
  
  // Start up screen three
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("City:");
  lcd.setCursor(0, 1);
  lcd.printf("%s", city.c_str());
  ThisThread::sleep_for(2s);

  lcd.clear();
};