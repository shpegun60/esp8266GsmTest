#ifndef SIM_TCP_CLIENT_SOCKET_H
#define SIM_TCP_CLIENT_SOCKET_H

#include "simbase.h"
#include "itcpclientsocket.h"

class simData
{
    static constexpr int buffSize = 100;
public:
    // set GSM PIN, if any
    char pinCode[buffSize] = "5548";

    // Your GPRS credentials, if any
    char apn[buffSize] = "internet";
    char gprsUser[buffSize] = "";
    char gprsPass[buffSize] = "";

    // modem baudrate
    int32_t baudRate = 57600;
};

template <class AT>
class simTcpClientSocket : public ITcpClientSocket, public SimBase<AT>
{
public:
    static constexpr int try_cnt = 10;

    simTcpClientSocket(AT &stream_AT, int rstPin) 
        : SimBase<AT>(stream_AT, rstPin)
    {
        client = new TinyGsmClient(*this, 0);
    }


    ~simTcpClientSocket() 
    {
        delete client;
    }


    // INTERFACE --------------------------------------------------------------------------------
    inline bool connected() override 
    {
        if(client == nullptr) {
            DBG("client is nullptr");
            return false;
        }
        return client->connected();
    }


    int connect(const char *host, uint16_t port) override 
    {
        if(client == nullptr || host == nullptr ) {
            DBG("Connecting securely tfalsed, is nullptr", port);
            return 0;
        }

        int trying = try_cnt;

        DBG("Connecting securely to", port);

        while(!client->connect(host, port)) {
            DBG("... failed");
            --trying;

            DBG("Connecting securely again to", port, " counter: ", trying);

            if(trying == 0) {
                DBG("Connecting securely failed, break!!!");
                return 0;
            }
        }

        return 1;
    }

    inline size_t write(const uint8_t *buf, size_t size) override
    {
        if(client == nullptr || buf == nullptr) {
            DBG("Write falsed, is nullptr");
            return 0;
        }

        return client->write(buf, size);
    }



    inline int read(uint8_t *buf, size_t size) override
    {
        if(client == nullptr || buf == nullptr) {
            DBG("READ falsed, is nullptr");
            return 0;
        }

        return client->read(buf, size);
    }

private:
    TinyGsmClient * client = nullptr;
};

#endif /* SIM_TCP_CLIENT_SOCKET_H */