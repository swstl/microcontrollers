#include "ThisThread.h"
#include "wifi.h"
#include "mbed.h"
#include "nsapi_types.h"
#include <cstdio>
#include <cstring>
#include <string>
#include "json.hpp"
#include "HTS221Sensor.h"

#define WAIT_TIME_MS 500ms
#define SDA PB_11
#define SCL PB_10

DevI2C com(SDA, SCL); //I2C2
HTS221Sensor sensor(&com); // HUMIDITY AND TEMPERATURE SENSOR

using json = nlohmann::json;

int main(){
    float humidity;
    float temperature;
    NetworkInterface *network = NetworkInterface::get_default_instance();
    nsapi_size_or_error_t result;
    SocketAddress address;
    const char key[] = "vdAZRAtGuc8T7RNnxTfs";
    char host[] = "192.168.39.121";
    constexpr int serverport = 9090;
    constexpr size_t response_size = 1000;
    constexpr size_t request_size = 300;
    constexpr size_t json_input_size = 100;
    static char response[response_size];
    static char request[request_size];
    static char json_input[json_input_size];

    if (sensor.init(NULL) != 0) {
        printf("Failed to init!\n");
    }

    if (sensor.enable() != 0) {
        printf("Failed to enable!\n");
    }

    if (!network){
        printf("default network interface failed\n");
        return -1;
    }
    printf("Trying to connect to the internet... \n");


    while (network->connect() != NSAPI_ERROR_OK)
        printf("internet connection failed, trying again...\n"); 
    while  (network->get_ip_address(&address) != NSAPI_ERROR_OK) 
        printf("failed to get local ip, trying again...\n"); 


    printf("Successful connection established\n");
    printf("Connected to ip: %s\n", address.get_ip_address());
  
    

    while (true) {

        sensor.get_humidity(&humidity);
        sensor.get_temperature(&temperature);

                
        snprintf(json_input, json_input_size,
            "{\"temperature\": %.2f, \"humidity\": %.2f}"
            "\r\n", temperature, humidity);
        
        snprintf(request, request_size, 
            "POST /api/v1/%s/telemetry HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "\r\n",
            key, host, strlen(json_input)-2);

        strcat(request, json_input);

        if (connect_to(host, network, address, serverport) != NSAPI_ERROR_OK)
            printf("Could not connect to %s\n\n", host);

        if(send_info(request) != NSAPI_ERROR_OK)
            printf("failed to send bytes\n");

        result = read_info(response, response_size);
        if (result != -1){
            response[result] = '\0';
            printf("Full response recieved: \n%s\n", response);
        }
        
        else
            printf("failed to receive response from %s\n", host);

        ThisThread::sleep_for(2000ms);
        printf("ip address: %s\n", address.get_ip_address());
        disconnect();
    }

        return 0;
}