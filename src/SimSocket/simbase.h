#ifndef SIM_BASE_H
#define SIM_BASE_H

#include "Stream.h"

/**************************************************************
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 * NOTE:
 * Some of the functions may be unavailable for your modem.
 * Just comment them out.
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_SIM5360
// #define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH


// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 57600


#include "TinyGsmClient.h"


template <class AT>
class SimBase : public TinyGsm
{
public:
    SimBase(AT &stream_AT, int rstPin)
        : TinyGsm(stream_AT)
    {
        m_stream_AT = &stream_AT;
        DBG("\n************************************************************");
        DBG(" SIM BASE INIT!!! ");
        DBG("************************************************************");
        
        pinMode(rstPin, OUTPUT);
        this->m_rstPin = rstPin;
    }

    inline const int getRstPin() const {
        return m_rstPin;
    }

    inline AT& getATStream() {
        return *m_stream_AT;
    }

    int SimStart(const int start_ms, const char* const pinCode = NULL, int tryCnt = 10, const bool autoBaudrate = false, const uint32_t baudRate = 57600)
    {
        // !!!!!!!!!!!
        // Set your reset, enable, power pins here
        // !!!!!!!!!!!
        DBG("RESET Wait...");
        digitalWrite(m_rstPin, LOW);
        delay(start_ms);
        digitalWrite(m_rstPin, HIGH);

        // Set GSM module baud rate
        if(autoBaudrate) {
            TinyGsmAutoBaud(*m_stream_AT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
        } else {
            m_stream_AT->begin(baudRate);
        }
        

        // Restart takes quite some time
        // To skip it, call init() instead of restart()
        DBG("Initializing modem...");

        while(!this->restart()) {
            DBG("Failed to restart modem, delaying 10s and retrying, try cnt:", tryCnt);
            --tryCnt;

            if(tryCnt == 0) {
                DBG("Failed to restart modem, breaking!!!!!");
                return 1;
            }
        }
        

        String name = this->getModemName();
        DBG("Modem Name:", name);

        String modemInfo = this->getModemInfo();
        DBG(modemInfo);

        // Unlock your SIM card with a PIN if needed
        if (pinCode && this->getSimStatus() != 3) { 
            this->simUnlock(pinCode);
        }

        return 0;
    }

#if defined(TINY_GSM_MODEM_HAS_WIFI)
    void ModemConnectToWifi(const char* const wifiSSID, const char* const wifiPass, int tryCnt = 10)
    {
        DBG("WIFI: Setting SSID/password...");

        while(!this->networkConnect(wifiSSID, wifiPass)) {
            DBG("WIFI: fail try cnt:", tryCnt);
            delay(10000);
            --tryCnt;

            if(tryCnt == 0) {
                DBG("WIFI: fail, break!!!");
                return 1;
            }
        }

        DBG("WIFI: success");

        return 0;
    }
#endif /* defined(TINY_GSM_MODEM_HAS_WIFI) */


#if defined(TINY_GSM_MODEM_HAS_GPRS)
    int ModemConnectToGPRS(const char* apn, const char* gprsUser = NULL, const char* gprsPass = NULL, const int tryCnt = 10)
    {
        int tryCnt_tmp = tryCnt;

        #if defined(TINY_GSM_MODEM_XBEE)
            // The XBee must run the gprsConnect function BEFORE waiting for network!
            this->gprsConnect(apn, gprsUser, gprsPass);
        #endif /* defined(TINY_GSM_MODEM_XBEE) */

        // CONNECTING TO NETWORK ------------------------------------------------------------------------
        DBG("GPRS: Waiting for network...");

        while(!this->waitForNetwork(600000L, true)) {
            delay(10000);
            --tryCnt_tmp;

            DBG("GPRS: Waiting for network... try cnt", tryCnt_tmp);

            if(tryCnt_tmp == 0) {
                DBG("GPRS: Waiting for network... break!!!");
                return 1;
            }
        }

        if (this->isNetworkConnected()) { 
            DBG("GPRS: Network connected"); 
        }


        // CONNECTING TO GPRS ------------------------------------------------------------------------

        DBG("GPRS: Connecting to", apn);

        tryCnt_tmp = tryCnt;
        while(!this->gprsConnect(apn, gprsUser, gprsPass)) {
            delay(10000);
            --tryCnt_tmp;

            DBG("GPRS: FAIL Connecting to", apn, " try cnt: ", tryCnt_tmp);
            if(tryCnt_tmp == 0) {
                DBG("GPRS: FAIL Connecting to", apn, " break!!!");
                return 1;
            }
        }

        bool res = this->isGprsConnected();
        DBG("GPRS status:", res ? "connected" : "not connected");

        String ccid = this->getSimCCID();
        DBG("CCID:", ccid);

        String imei = this->getIMEI();
        DBG("IMEI:", imei);

        String imsi = this->getIMSI();
        DBG("IMSI:", imsi);

        String cop = this->getOperator();
        DBG("Operator:", cop);

        IPAddress local = this->localIP();
        DBG("Local IP:", local);

        int csq = this->getSignalQuality();
        DBG("Signal quality:", csq);
        
        return 0;
    }
#endif /* defined(TINY_GSM_MODEM_HAS_GPRS) */



private:
    int m_rstPin = 0;
    AT * m_stream_AT = nullptr;
};


#endif /* SIM_BASE_H */