#include "wifi.h"
#include "NetworkInterface.h"
#include "ThisThread.h"
#include "mbed_retarget.h"
#include "mstd_iterator"
#include "nsapi_types.h"
#include <cstddef>
#include <cstdio>

static TCPSocket socket;
static Socket *sct = &socket;
static nsapi_size_or_error_t response;
static char *host;


nsapi_size_t connect_to(char* web, NetworkInterface *network, SocketAddress &adr, const int &port){
    
    host = web;
    do {
        socket.set_timeout(500);
        if (socket.open(network) != NSAPI_ERROR_OK)
            printf("TCPSocket failed\n");

        response = network->gethostbyname(web, &adr);
        if (response != NSAPI_ERROR_OK){
            printf("failed to get hosts ip adr\n");   
            return -1;
        }

        adr.set_port(port);
        response = socket.connect(adr);
        if (response != NSAPI_ERROR_OK){
            printf("failed to connect to the server\n");
            return -2;        
        }

    }while (response != NSAPI_ERROR_OK);

    printf("Successfully connected to %s\n\n", web);

    return 0;
}


nsapi_size_t send_info(char *request){
    if (response != NSAPI_ERROR_OK) {           
        printf("No open connections detected, make sure to connect to a host first!\n");
        return -1;
    }

    nsapi_size_t send_message = strlen(request);
    nsapi_size_or_error_t actually_sent = 0;

    printf("Sending: \n%s\n", request);
    printf("Total bytes to send: %i\n", send_message);

    while (send_message){
        actually_sent = sct->send(request + actually_sent, send_message);
        printf("bytes sent: %i\n", actually_sent); 
        send_message -= actually_sent;

        if (actually_sent < 0){
            printf("Error occured while sending message\n");
            return actually_sent;
        }
    }

    return 0;
}

nsapi_size_t read_info(char *buffer, size_t size_of_buffer){
    memset(buffer, 0, size_of_buffer);
    int expected_bytes = size_of_buffer;
    int presented_bytes = 0;

    while (expected_bytes > 0) {
        response = sct->recv(buffer + presented_bytes, size_of_buffer);
    
        if (response < 0)
            return -1;

        if (response == 0)
            break;

        expected_bytes -= response;
        presented_bytes += response;
        ThisThread::sleep_for(500ms); // making it more dramatic....

    }
    return presented_bytes;
}

nsapi_size_t disconnect(){
    return socket.close();
}