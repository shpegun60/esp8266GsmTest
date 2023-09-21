#ifndef SIM_TESTER_H
#define SIM_TESTER_H

#include "simbase.h"


/*
 * Tests enabled
 */
#define TINY_GSM_TEST_GPRS true
#define TINY_GSM_TEST_WIFI true
#define TINY_GSM_TEST_TCP true
#define TINY_GSM_TEST_SSL false
#define TINY_GSM_TEST_CALL false
#define TINY_GSM_TEST_SMS false
#define TINY_GSM_TEST_USSD true
#define TINY_GSM_TEST_BATTERY true
#define TINY_GSM_TEST_TEMPERATURE true
#define TINY_GSM_TEST_GSM_LOCATION true
#define TINY_GSM_TEST_NTP true
#define TINY_GSM_TEST_TIME true
#define TINY_GSM_TEST_GPS true
// disconnect and power down modem after tests
#define TINY_GSM_POWERDOWN true




class SimTestdata
{
    static constexpr int buffSize = 100;
public:

    // set GSM PIN, if any
    char pinCode[buffSize] = "5548";

    // Set phone numbers, if you want to test SMS and Calls
    char smsTarget[buffSize] = "+380960755746";
    char callTarget[buffSize] = "+380960755746";

    // set USSD for test 
    String USSD_test = "*111#";

    // Your GPRS credentials, if any
    char apn[buffSize] = "internet";
    char gprsUser[buffSize] = "";
    char gprsPass[buffSize] = "";

    // Your WiFi connection credentials, if applicable
    char wifiSSID[buffSize] = "YourSSID";
    char wifiPass[buffSize] = "YourWiFiPass";

    //// Server details to test TCP/SSL
    // const int     port = 80;
    // const int     securePort = 80;
    // const char server[buffSize]   = "vsh.pp.ua";
    // const char resource[buffSize] = "/TinyGSM/logo.txt";

    int port = 8875;
    int securePort = 8875;
    char server[buffSize]   = "185.115.36.220";
    char resource[buffSize] = "/calc_cl?red=100";
};

template <class AT>
class SimTester : public SimBase<AT>
{
public:

    SimTester(AT &stream_AT, int rstPin)
        : SimBase<AT>(stream_AT, rstPin)
    {
        DBG("\n************************************************************");
        DBG(" SIM TEST INIT!!! ");
        DBG("************************************************************");

        DBG("\n\n************************BLA, BLA, BLA************************************");
    }

    int simTest(SimTestdata &testdata, int tryCnt)
    {
        int countNotPassed = 0;
        bool res = false;
        (void)res;

        // SIM-CARD START -----------------------------------------------------------------------
        if(this->SimStart(6000, testdata.pinCode, tryCnt, true)) {
            return 1;
        }

        // test wi-fi ---------------------------------------------------
    #if TINY_GSM_TEST_WIFI && defined TINY_GSM_MODEM_HAS_WIFI
        countNotPassed += this->ModemConnectToWifi(testdata.wifiSSID, testdata.wifiPass, tryCnt);
    #endif /*TINY_GSM_TEST_WIFI && defined TINY_GSM_MODEM_HAS_WIFI*/


        // TEST NETWORK & GPRS -----------------------------------------------------------------------
    #if TINY_GSM_TEST_GPRS && defined TINY_GSM_MODEM_HAS_GPRS
        this->ModemConnectToGPRS(testdata.apn, testdata.gprsUser, testdata.gprsPass, tryCnt);
    #endif /* TINY_GSM_TEST_GPRS && defined TINY_GSM_MODEM_HAS_GPRS */


        // TEST USSD -----------------------------------------------------------------------
    #if TINY_GSM_TEST_USSD && defined TINY_GSM_MODEM_HAS_SMS
        String ussd_balance = this->sendUSSD(testdata.USSD_test);
        DBG("Balance (USSD):", ussd_balance);
    #endif /* TINY_GSM_TEST_USSD && defined TINY_GSM_MODEM_HAS_SMS */


        // TEST TCP CONNECTION -----------------------------------------------------------------------
    #if TINY_GSM_TEST_TCP && defined TINY_GSM_MODEM_HAS_TCP
        TinyGsmClient client(*this, 0);
        
        DBG("Connecting to", testdata.server);

        if (!client.connect(testdata.server, testdata.port)) {
            DBG("... failed");
        } else {
            // Make a HTTP GET request:
            client.print(String("GET ") + testdata.resource + " HTTP/1.0\r\n");
            client.print(String("Host: ") + testdata.server + "\r\n");
            client.print("Connection: close\r\n\r\n");

            // Wait for data to arrive
            uint32_t start = millis();
            while (client.connected() && !client.available() &&
                    millis() - start < 30000L) {
                delay(100);
            };

            // Read data
            start          = millis();
            char logo[640] = {
                '\0',
            };

            int read_chars = 0;
            while (client.connected() && millis() - start < 10000L) {
                while (client.available()) {
                    logo[read_chars]     = client.read();
                    logo[read_chars + 1] = '\0';
                    read_chars++;
                    start = millis();
                }
            }

            DBG(logo);
            DBG("#####  RECEIVED:", strlen(logo), "CHARACTERS");
            client.stop();
        }
    #endif /* TINY_GSM_TEST_TCP && defined TINY_GSM_MODEM_HAS_TCP */

         // TEST SECURE TCP CONNECTION -----------------------------------------------------------------------
    #if TINY_GSM_TEST_SSL && defined TINY_GSM_MODEM_HAS_SSL
        TinyGsmClientSecure secureClient(*this, 1);
        
        DBG("Connecting securely to", testdata.server);
        if (!secureClient.connect(testdata.server, testdata.securePort)) {
            DBG("... failed");
        } else {
            // Make a HTTP GET request:
            secureClient.print(String("GET ") + testdata.resource + " HTTP/1.0\r\n");
            secureClient.print(String("Host: ") + testdata.server + "\r\n");
            secureClient.print("Connection: close\r\n\r\n");

            // Wait for data to arrive
            uint32_t startS = millis();
            while (secureClient.connected() && !secureClient.available() &&
                    millis() - startS < 30000L) {
                delay(100);
            };

            // Read data
            startS          = millis();
            char logoS[640] = {
                '\0',
            };
            
            int read_charsS = 0;
            while (secureClient.connected() && millis() - startS < 10000L) {
                while (secureClient.available()) {
                    logoS[read_charsS]     = secureClient.read();
                    logoS[read_charsS + 1] = '\0';
                    read_charsS++;
                    startS = millis();
                }
            }
            DBG(logoS);
            DBG("#####  RECEIVED:", strlen(logoS), "CHARACTERS");
            secureClient.stop();
        }
    #endif /* TINY_GSM_TEST_SSL && defined TINY_GSM_MODEM_HAS_SSL */

    #if TINY_GSM_TEST_CALL && defined TINY_GSM_MODEM_HAS_CALLING
        DBG("Calling:", testdata.callTarget);

        // This is NOT supported on M590
        res = this->callNumber(testdata.callTarget);
        DBG("Call:", res ? "OK" : "fail");

        if (res) {
            delay(1000L);

            // Play DTMF A, duration 1000ms
            this->dtmfSend('A', 1000);

            // Play DTMF 0..4, default duration (100ms)
            for (char tone = '0'; tone <= '4'; tone++) { 
                this->dtmfSend(tone); 
            }

            delay(5000);

            res = this->callHangup();
            DBG("Hang up:", res ? "OK" : "fail");
        }
    #endif /* TINY_GSM_TEST_CALL && defined TINY_GSM_MODEM_HAS_CALLING */

    #if TINY_GSM_TEST_SMS && defined TINY_GSM_MODEM_HAS_SMS
        res = this->sendSMS(testdata.smsTarget, String("Hello from ") + this->getIMEI());
        DBG("SMS:", res ? "OK" : "fail");

        // This is only supported on SIMxxx series
        res = this->sendSMS_UTF8_begin(testdata.smsTarget);
        if (res) {
            auto stream = this->sendSMS_UTF8_stream();
            stream.print(F("Привіііт! Print number: "));
            stream.print(595);
            res = this->sendSMS_UTF8_end();
        }
        DBG("UTF8 SMS:", res ? "OK" : "fail");

    #endif

    #if TINY_GSM_TEST_GSM_LOCATION && defined TINY_GSM_MODEM_HAS_GSM_LOCATION
        float lat      = 0;
        float lon      = 0;
        float accuracy = 0;
        int   year     = 0;
        int   month    = 0;
        int   day      = 0;
        int   hour     = 0;
        int   min      = 0;
        int   sec      = 0;
        for (int8_t i = 15; i; i--) {
            DBG("Requesting current GSM location");
            if (this->getGsmLocation(&lat, &lon, &accuracy, &year, &month, &day, &hour,
                                        &min, &sec)) {
                DBG("Latitude:", String(lat, 8), "\tLongitude:", String(lon, 8));
                DBG("Accuracy:", accuracy);
                DBG("Year:", year, "\tMonth:", month, "\tDay:", day);
                DBG("Hour:", hour, "\tMinute:", min, "\tSecond:", sec);
                break;
            } else {
                DBG("Couldn't get GSM location, retrying in 15s.");
                delay(15000L);
            }
        }
        DBG("Retrieving GSM location again as a string");
        String location = this->getGsmLocation();
        DBG("GSM Based Location String:", location);
    #endif /* TINY_GSM_TEST_GSM_LOCATION && defined TINY_GSM_MODEM_HAS_GSM_LOCATION */

    #if TINY_GSM_TEST_GPS && defined TINY_GSM_MODEM_HAS_GPS
        DBG("Enabling GPS/GNSS/GLONASS and waiting 15s for warm-up");
        this->enableGPS();
        delay(15000L);
        float lat2      = 0;
        float lon2      = 0;
        float speed2    = 0;
        float alt2      = 0;
        int   vsat2     = 0;
        int   usat2     = 0;
        float accuracy2 = 0;
        int   year2     = 0;
        int   month2    = 0;
        int   day2      = 0;
        int   hour2     = 0;
        int   min2      = 0;
        int   sec2      = 0;
        for (int8_t i = 15; i; i--) {
        DBG("Requesting current GPS/GNSS/GLONASS location");
            if (this->getGPS(&lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                                &year2, &month2, &day2, &hour2, &min2, &sec2)) {
                DBG("Latitude:", String(lat2, 8), "\tLongitude:", String(lon2, 8));
                DBG("Speed:", speed2, "\tAltitude:", alt2);
                DBG("Visible Satellites:", vsat2, "\tUsed Satellites:", usat2);
                DBG("Accuracy:", accuracy2);
                DBG("Year:", year2, "\tMonth:", month2, "\tDay:", day2);
                DBG("Hour:", hour2, "\tMinute:", min2, "\tSecond:", sec2);
                break;
            } else {
                DBG("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
                delay(15000L);
            }
        }
        DBG("Retrieving GPS/GNSS/GLONASS location again as a string");
        String gps_raw = this->getGPSraw();
        DBG("GPS/GNSS Based Location String:", gps_raw);
        DBG("Disabling GPS");
        this->disableGPS();
    #endif /* TINY_GSM_TEST_GPS && defined TINY_GSM_MODEM_HAS_GPS */

    #if TINY_GSM_TEST_NTP && defined TINY_GSM_MODEM_HAS_NTP
        DBG("Asking modem to sync with NTP");
        this->NTPServerSync("132.163.96.5", 20);
    #endif /* TINY_GSM_TEST_NTP && defined TINY_GSM_MODEM_HAS_NTP */

    #if TINY_GSM_TEST_TIME && defined TINY_GSM_MODEM_HAS_TIME
        int   year3    = 0;
        int   month3   = 0;
        int   day3     = 0;
        int   hour3    = 0;
        int   min3     = 0;
        int   sec3     = 0;
        float timezone = 0;
        for (int8_t i = 5; i; i--) {
            DBG("Requesting current network time");
            if (this->getNetworkTime(&year3, &month3, &day3, &hour3, &min3, &sec3,
                                        &timezone)) {
                DBG("Year:", year3, "\tMonth:", month3, "\tDay:", day3);
                DBG("Hour:", hour3, "\tMinute:", min3, "\tSecond:", sec3);
                DBG("Timezone:", timezone);
                break;
            } else {
                DBG("Couldn't get network time, retrying in 15s.");
                delay(15000L);
            }
        }
        DBG("Retrieving time again as a string");
        String time = this->getGSMDateTime(DATE_FULL);
        DBG("Current Network Time:", time);
    #endif /* TINY_GSM_TEST_TIME && defined TINY_GSM_MODEM_HAS_TIME */

    #if TINY_GSM_TEST_BATTERY && defined TINY_GSM_MODEM_HAS_BATTERY
        uint8_t  chargeState = -99;
        int8_t   percent     = -99;
        uint16_t milliVolts  = -9999;
        this->getBattStats(chargeState, percent, milliVolts);
        DBG("Battery charge state:", chargeState);
        DBG("Battery charge 'percent':", percent);
        DBG("Battery voltage:", milliVolts / 1000.0F);
    #endif /* TINY_GSM_TEST_BATTERY && defined TINY_GSM_MODEM_HAS_BATTERY */

    #if TINY_GSM_TEST_TEMPERATURE && defined TINY_GSM_MODEM_HAS_TEMPERATURE
        float temp = this->getTemperature();
        DBG("Chip temperature:", temp);
    #endif /* TINY_GSM_TEST_TEMPERATURE && defined TINY_GSM_MODEM_HAS_TEMPERATURE */

    #if TINY_GSM_POWERDOWN

        #if TINY_GSM_TEST_GPRS && defined TINY_GSM_MODEM_HAS_GPRS
            this->gprsDisconnect();
            delay(5000L);
            if (!this->isGprsConnected()) {
                DBG("GPRS disconnected");
            } else {
                DBG("GPRS disconnect: Failed.");
            }
        #endif /* TINY_GSM_TEST_GPRS && defined TINY_GSM_MODEM_HAS_GPRS */

        #if TINY_GSM_TEST_WIFI && defined TINY_GSM_MODEM_HAS_WIFI
            this->networkDisconnect();
            DBG("WiFi disconnected");
        #endif /* TINY_GSM_TEST_WIFI && defined TINY_GSM_MODEM_HAS_WIFI */

        // Try to power-off (modem may decide to restart automatically)
        // To turn off modem completely, please use Reset/Enable pins
        this->poweroff();
        DBG("Poweroff.");
    #endif /* TINY_GSM_POWERDOWN */

        DBG("End of tests.");

        return countNotPassed;
    }

};

#endif /* SIM_TESTER_H */