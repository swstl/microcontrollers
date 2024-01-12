/**
 * @file   WifiThread.cpp
 * @authors Sander Wesstøl, Luka Vulovic
 */

#include "SocketAddress.h"
#include "mbed_retarget.h"
#include "wifi.h"
#include "WifiThread.h"
#include "certificate.h"
#include "TLSSocket.h"
#include "data.h"
#include "NetworkInterface.h"
#include "ThisThread.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <json.hpp>

#define size_of_screen 16           // the number of horisontal groups of pixels on the display
#define Newsfeed_offset 35          // newsfeed location from pointer to each "<item>" in response
#define Temperature_offset 17       // temperature location from "air_temperature" in response
#define Temperature_placement 3     // number of objects down the "correct" temperature is located
#define Symbol_code_offset 41       // location of day description from "next_1_hours" in response
#define loop_of_activation 300      // number of loops (1s) before weather and news request (5 min) 

using json = nlohmann::json;

void controller(Data *data){
    int num_of_loops = loop_of_activation;  // activate 1st loop
    json raw_info;
    NetworkInterface *network = NetworkInterface::get_default_instance();
    nsapi_size_or_error_t result;
    SocketAddress address;
    static char response[3000];

    const char geoloc[] = "api.ipgeolocation.io";
    static const char request_geolocation[] = 
                                    "GET /timezone?apiKey=9b13088404574027bd8bbd0db74f22ad HTTP/1.1\r\n"
                                    "Host: api.ipgeolocation.io\r\n"
                                    "Connection: close\r\n"
                                    "\r\n";

    static const char request_geolocation_with_location[] = 
                                    "GET /timezone?apiKey=9b13088404574027bd8bbd0db74f22ad&location=%s HTTP/1.1\r\n"
                                    "Host: api.ipgeolocation.io\r\n"
                                    "Connection: close\r\n"
                                    "\r\n";

    const char news[] = "feeds.bbci.co.uk";
    static const char request_news[] = 
                                    "GET /news/world/rss.xml HTTP/1.1\r\n"
                                    "Host: feeds.bbci.co.uk\r\n"
                                    "Connection: close\r\n"
                                    "\r\n";

    const char weather[] = "api.met.no";
    static const char request_weather[] =
                                    "GET /weatherapi/locationforecast/2.0/compact?lat=%s&lon=%s HTTP/1.1\r\n"
                                    "Host: api.met.no\r\n"
                                    "User-Agent: schoolstudent\r\n"
                                    "Connection: close\r\n"
                                    "\r\n";


    // connects to the local network
    if (!network){
        printf("default network interface failed\n");
        while (true);
    }
    printf("Trying to connect to the internet... \n");

    while (network->connect() != NSAPI_ERROR_OK)
        printf("internet connection failed, trying again...\n"); 

    while  (network->get_ip_address(&address) != NSAPI_ERROR_OK) 
        printf("failed to get local ip, trying again...\n"); 

    printf("Successful connection established\n");
    printf("Connected to ip: %s\n", address.get_ip_address());



    // connect and recieve info from api.ipgeolocation.io
    connect_to(geoloc, network, address, geolocation_certificate, 443);
    if (send_info(request_geolocation) != NSAPI_ERROR_OK)
        printf("Failed to send request\n");
    if (read_info(response, sizeof(response)) < 0)
        printf("Failed to receive response\n");
    disconnect();

    convert_to_json(response, raw_info);
    // imidiently sets the RTC in the microcontroller to the given UNIX EPOCH TIME, with the additional offset
    set_time(raw_info["date_time_unix"].get<double>() + 3600*raw_info["timezone_offset_with_dst"].get<int>());
    insert_data_time_with_strings(data, raw_info);



    //indicates that default screen (along with some configuration) is ready to be displayed
    data->init = true;
    printf("Data is ready\n");



    while (true) {
        num_of_loops++;

        // Get newsfeed every 5 minutes
        if (num_of_loops >= loop_of_activation){

            connect_to(news, network, address, 80);

            if (send_info(request_news) != NSAPI_ERROR_OK)
                printf("Failed to send request\n");
            
            // should it fail to read the response from the website, it will retry the connection.
            if (read_info(response, sizeof(response)) < 0){
                printf("Failed to receive response, trying again.. \n");
                disconnect();
                continue;
                }
         
            disconnect();
        
            insert_data_news(data, response);  
        
        }



        // connect and recieve info from api.ipgeolocation.io
        if (data->location_change){
            char request[sizeof(request_geolocation_with_location) + 30];
            snprintf(request, sizeof(request_geolocation_with_location) + 30, request_geolocation_with_location, data->time.city.c_str());


            connect_to(geoloc, network, address, geolocation_certificate, 443);

            if (send_info(request) != NSAPI_ERROR_OK)
                printf("Failed to send request\n");
            
            // should it fail to read the response from the website, it will retry the connection.
            if (read_info(response, sizeof(response)) < 0){
                printf("Failed to receive response, trying again.. \n");
                disconnect();
                continue;
            }
            
            disconnect();

            convert_to_json(response, raw_info);
            // changes the RTC in the microcontroller to the new UNIX EPOCH TIME, with its additional offset.
            set_time(raw_info["date_time_unix"].get<double>() + 3600*raw_info["timezone_offset_with_dst"].get<int>());
            insert_data_time_with_integers(data, raw_info);

        } 



        // Get weather report for current clockhour each 5 min, or right after a new location has been set
        if(num_of_loops >= loop_of_activation || data->location_change){
            connect_to(weather, network, address, met_certificate, 443);
            char request[sizeof(request_weather)+30];
            snprintf(request, sizeof(request_weather)+30, 
                     request_weather, 
                     data->time.latitude.c_str(), 
                     data->time.longitude.c_str()); // inserts coordinates to request

            if (send_info(request) != NSAPI_ERROR_OK){
                printf("Failed to send request\n");\
            }
            
            // should it fail to read the response from the website, it will retry the connection.
            if (read_info(response, sizeof(response)) < 0){
                printf("Failed to receive response, trying again.. \n");
                disconnect();
                continue;
            }
    
            disconnect();
            
            insert_data_weather(data, response);
            
            num_of_loops = 0;
            data->location_change = false;
        }

        // There is on need for this thread to consume more processor power than needed. 
        // Though it still needs to be updated when a new location is set.
        ThisThread::sleep_for(1s);
    }
}



// parses a text to json (if any json data is found);
void convert_to_json(char *text, nlohmann::json &jsn){

    auto json_start = strchr(text, '{');
    auto json_end = strrchr(text, '}');

    *(json_end+1) = '\0';

    jsn = json::parse(json_start);

    if (jsn.is_discarded())
        printf("invalid json");

}

/** insert_data_weather
*   Inserts weather report into data
*   The weather response returns a vary long json formatted text,
*   But rather than assigning 30k bytes to the response,
*   i decided to seach for the words in a smaller response instead.
*/
void insert_data_weather(Data *data, const char *text){

    // point to the object of the json list that we want
    auto *item = strstr(text, "air_temperature")+Temperature_offset;
    for (int i = 0; i < Temperature_placement; i++) {
    item = strstr(item, "air_temperature")+Temperature_offset;
    }
    data->weather.degrees.clear();
    data->weather.description.clear();


    // insert the degrees which stops at ,
    while (*item != ','){
        data->weather.degrees.push_back(*item);
        item++;
    }


    // find weather description belonging to that object
    item = strstr(item, "next_1_hours")+Symbol_code_offset;
    while (*item != '\"'){

        if (*item == '_')
            data->weather.description.push_back(' ');
        else
            data->weather.description.push_back(*item);
        
        item++;
    
    }


    // remove last word in weather description IF it doesn't fit the screen:
    if (data->weather.description.size() > size_of_screen){
        for (auto it = data->weather.description.end(); it != data->weather.description.begin(); it --){
            if (*it == ' '){
                data->weather.description.erase(it, data->weather.description.end());
            }
        }
    }
}



// Inserts json data into data class;
void insert_data_time_with_strings(Data *data, nlohmann::json &jsn){ 
    data->time.START_UNIX_RTC = jsn["date_time_unix"].get<time_t>();
    data->time.latitude = jsn["geo"]["latitude"].get<std::string>();
    data->time.longitude = jsn["geo"]["longitude"].get<std::string>();
    data->time.city = jsn["geo"]["city"].get<std::string>();
}


// Inserts json data into data class;
void insert_data_time_with_integers(Data *data, nlohmann::json &jsn){ 
    data->time.latitude = to_string(jsn["geo"]["latitude"].get<int>());
    data->time.longitude = to_string(jsn["geo"]["longitude"].get<int>());
}



// inserts news feed into data class
void insert_data_news(Data *data, const char *text){
    auto *Newsfeed = strstr(text, "<item>")+Newsfeed_offset;
    data->news.newsfeed1.clear();
    data->news.newsfeed2.clear();
    data->news.newsfeed3.clear();

    // 1st newsfeed
    while (*Newsfeed != ']'){
        data->news.newsfeed1.push_back(*Newsfeed);
        Newsfeed++;
    }

    // 2nd newsfeed
    Newsfeed = strstr(Newsfeed, "<item>")+Newsfeed_offset;
    while (*Newsfeed != ']'){
        data->news.newsfeed2.push_back(*Newsfeed);
        Newsfeed++;
    }

    // 3rd newsfeed
    Newsfeed = strstr(Newsfeed, "<item>")+Newsfeed_offset;
    while (*Newsfeed != ']'){
        data->news.newsfeed3.push_back(*Newsfeed);
        Newsfeed++;
    }

    // make all 3 newsfeeds into one long newsfeed, separated by " n: "
    data->news.whole_newsfeed.append("  1: ");
    data->news.whole_newsfeed.append(data->news.newsfeed1);

    data->news.whole_newsfeed.append("  2: ");
    data->news.whole_newsfeed.append(data->news.newsfeed2);

    data->news.whole_newsfeed.append("  3: ");
    data->news.whole_newsfeed.append(data->news.newsfeed3);
}