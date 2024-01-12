#include "ThisThread.h"
#include "wifi.h"
#include "mbed.h"
#include "nsapi_types.h"
#include <cstdio>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

int main(){
    NetworkInterface *network = NetworkInterface::get_default_instance();
    nsapi_size_or_error_t result;
    SocketAddress address;
    char host[] = "www.mocky.io";
    constexpr int size = 2001;
    static char response[size];
    std::string text_input;
    const char request[] = "GET /v2/5e37e64b3100004c00d37d03 HTTP/1.1\r\n"
                           "Host: www.mocky.io\r\n"
                           "Connection: close\r\n"
                           "\r\n";

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


    if (connect_to(host, network, address) != NSAPI_ERROR_OK)
        printf("Could not connect to %s\n\n", host);


    if(send_info(request) != NSAPI_ERROR_OK)
        printf("failed to send bytes\n");

    result = read_info(response, size);
    if (result != -1)
        response[result] = '\0';

    else
        printf("failed to receive response from %s\n", host);

    disconnect();
    printf("disconnected from server\n");

    printf("Full response recieved: \n%s\n", response);
    printf("Information given by %s: \n", host);
    
    for (int i = 0; i < strlen(response); i++){
        if (response[i] == '{'){
            text_input = std::string(response).erase(0, i);
            break;
        }
    }

    json info = json::parse(text_input);
    printf("First name: %s\n", info["first name"].get<std::string>().c_str());
    printf("Last name: %s\n", info["last name"].get<std::string>().c_str());
    printf("Age: %i\n", info["age"].get<int>());

    while (true) {
        ThisThread::sleep_for(10000ms);
        printf("\"%s\"s ip address: %s\n",host, address.get_ip_address());
    }

        return 0;
}