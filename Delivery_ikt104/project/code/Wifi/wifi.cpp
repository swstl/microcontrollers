/**
 * @file   wifi.cpp
 * @authors Sander Wesstøl, Erlend Tregde
 */

#include "wifi.h"
#include "Socket.h"
#include "TCPSocket.h"
#include "TLSSocket.h"
#include "data.h"
#include "NetworkInterface.h"
#include "ThisThread.h"
#include "mbed_retarget.h"
#include "mstd_iterator"
#include "nsapi_types.h"
#include <cstddef>
#include <cstdio>

Socket *sct = nullptr;
static nsapi_size_or_error_t response = NSAPI_ERROR_NO_CONNECTION;

// connect using https
nsapi_size_or_error_t connect_to(const char* web, NetworkInterface *network, SocketAddress &adr, const char *certificate, const int &port){
    auto *socket_https = new TLSSocket;
    sct = socket_https;

    do {
        socket_https->set_timeout(1000);
        if (socket_https->open(network) != NSAPI_ERROR_OK)
            printf("TLSsocket_https failed\n");

        response = network->gethostbyname(web, &adr);
        if (response != NSAPI_ERROR_OK){
            printf("failed to get hosts ip adr\n");  
            return response;
        }

        adr.set_port(port);

        response = socket_https->set_root_ca_cert(certificate);
        if (response  != NSAPI_ERROR_OK){
            printf("Failed to set certificate: %s", web);
            return response;
        }

        socket_https->set_hostname(web);

        response = socket_https->connect(adr);
        if (response != NSAPI_ERROR_OK){
            printf("failed to connect to the server: %i\n", response);
            return response;        
        }

    }while (response != NSAPI_ERROR_OK);

    printf("Successfully connected to %s\n\n", web);

    return NSAPI_ERROR_OK;
}

 
 // connect using http
nsapi_size_or_error_t connect_to(const char* web, NetworkInterface *network, SocketAddress &adr, const int &port){
    auto socket_http = new TCPSocket;
    sct = socket_http;

    do {
        socket_http->set_timeout(1000);
        if (socket_http->open(network) != NSAPI_ERROR_OK)
            printf("TCPsocket_https failed\n");

        response = network->gethostbyname(web, &adr);
        if (response != NSAPI_ERROR_OK){
            printf("failed to get hosts ip adr\n");  
            return response;
        }

        adr.set_port(port);
        response = socket_http->connect(adr);
        if (response != NSAPI_ERROR_OK){
            printf("failed to connect to the server\n");
            return response;        
        }

    }while (response != NSAPI_ERROR_OK);

    printf("Successfully connected to %s\n\n", web);

    return NSAPI_ERROR_OK;
}


nsapi_size_or_error_t send_info(const char *request){

    if (response != NSAPI_ERROR_OK) {           
        printf("No open connections detected, make sure to connect to a host first!\n");
        return response;
    }

    nsapi_size_t send_message = strlen(request);
    nsapi_size_or_error_t actually_sent = 0;

    printf("Sending: %s", request);
    printf("Total bytes to send: %i\n", send_message);

    while (send_message){
        actually_sent = sct->send(request + actually_sent, send_message);
        send_message -= actually_sent;

        if (actually_sent < 0){
            printf("Error occured while sending message\n");
            return actually_sent;
        }
    }

    return NSAPI_ERROR_OK;
}


/** read_info:
*   if the buffer isn't big enough for the whole response,
*   it will just stop after filling the buffer with information. 
*/
nsapi_size_or_error_t read_info(char *buffer, size_t size_of_buffer){

    // clears the response buffer before inserting data
    memset(buffer, 0, size_of_buffer);

    int expected_bytes = size_of_buffer;
    int presented_bytes = 0;

    while (expected_bytes > 0) {
        response = sct->recv(buffer + presented_bytes, size_of_buffer - presented_bytes);
    
        // if response is negative the "read_info" failed
        if (response < 0)
            return response;

        // if response is 0 it's a successfull read
        if (response == 0)
            break;

        expected_bytes -= response;
        presented_bytes += response;
    }

    return presented_bytes;
}

void disconnect(){
    if (!sct){
        printf("no connection found\n");
    }

    sct->close();
    delete sct;
    sct = nullptr;
}