/**
 * @file   wifi.h
 * @authors Sander Wesstøl, Erlend Tregde
 */

#ifndef wifi
#define wifi

#include "TLSSocket.h"
#include "data.h"
#include "NetworkInterface.h"
#include "mbed.h"
#include "nsapi_types.h"

//connect using https
nsapi_size_or_error_t connect_to(const char*, NetworkInterface*, SocketAddress&, const char*, const int&);

//connect using http
nsapi_size_or_error_t connect_to(const char*, NetworkInterface*, SocketAddress&, const int&);

//send requests
nsapi_size_or_error_t send_info(const char*);

//read response
nsapi_size_or_error_t read_info(char*, size_t);

//disconnect socket
void disconnect();

#endif //wifi