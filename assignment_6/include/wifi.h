#ifndef wifi
#define wifi

#include "NetworkInterface.h"
#include "mbed.h"
#include "nsapi_types.h"

nsapi_size_t connect_to(char*, NetworkInterface*, SocketAddress&);

nsapi_size_t send_info(const char*);

nsapi_size_t read_info(char*, size_t);

nsapi_size_t disconnect();

#endif //wifi