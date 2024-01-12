/**
 * @file   WifiThread.h
 * @authors Sander Wesstøl, Luka Vulovic
 */

#ifndef WifiThread
#define WifiThread

#include "data.h"
#include <json.hpp>

void controller(Data *data);

void insert_data_weather(Data *data, const char *text);

/** since ipgeolocation decided to have longitude and latitude as different variable types when you add location, 
*   i couldn't use the same function twice. 
*/
void insert_data_time_with_strings(Data *data, nlohmann::json &jsn);
void insert_data_time_with_integers(Data *data, nlohmann::json &jsn); 

void insert_data_news(Data *data, const char *text);

void convert_to_json(char *text, nlohmann::json &jsn);

#endif //WifiThread