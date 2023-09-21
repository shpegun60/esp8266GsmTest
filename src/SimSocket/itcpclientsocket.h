#ifndef ITCP_CLIENT_SOCKET_H
#define ITCP_CLIENT_SOCKET_H

#include <stddef.h>
#include <stdint.h>

class ITcpClientSocket
{
public:
    virtual bool connected() = 0;
    virtual int connect(const char* host, uint16_t port) = 0;
    virtual size_t write(const uint8_t *buf, size_t size) = 0;
    virtual int read(uint8_t* buf, size_t size) = 0;
};



#endif /* ITCP_CLIENT_SOCKET_H */
