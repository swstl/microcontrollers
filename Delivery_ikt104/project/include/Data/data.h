/**
 * @file   data.h
 * @authors Sander Wesstøl, Linor Ujkani
 */

#ifndef __Data__
#define __Data__

#include "mbed.h"
#include <iostream>
#include <ctime>
#include <string>


/** We decided not to use mutexes in this assignment as it was not really necessary,
 *   since the main thread mostly just reads from the data retreived by the 2nd thread,
 *   and at the times the main thread actually writes to the data addresses, 
 *   the 2nd thread (wifithread) isn't writing to that same address. (not that it's hard to take use of mutexes).
 */

class Data{
    public:
    bool init = false;
    bool location_change = false;


    //information from https://api.ipgeolocation.io
    struct {
        time_t START_UNIX_RTC = 0; // unchanged = 0
        std::string latitude;
        std::string longitude;
        std::string city;
    }time;



    //information from http://feeds.bbci.co.uk
    struct {
        std::string newsfeed1;
        std::string newsfeed2;
        std::string newsfeed3;
        std::string whole_newsfeed;
    }news;



    //information from https://api.met.no
    struct{
        std::string description;
        std::string degrees;
    }weather;
};


#endif //__Data__