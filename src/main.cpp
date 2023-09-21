#include <Arduino.h>

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

// // Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

#include <SoftwareSerial.h>
SoftwareSerial SerialAT(14, 12); // RX, TX

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// #ifdef DUMP_AT_COMMANDS
// #include <StreamDebugger.h>
// StreamDebugger debugger(SerialAT, SerialMon);
// TinyGsm        modem(debugger);
// #else
// //TinyGsm        modem(SerialAT);
// #endif

#include "simtest.h"
#include "simtcpclientsocket.h"

int port = 8875;
char host[] = "185.115.36.220";

simData initData;
simTcpClientSocket<SoftwareSerial> *client;

const uint8_t TXbuf[] = "HELLO SERVER!!!";
char RXbuf[255] = {};

void setup()
{
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // test-----------------------------------------------
  // SimTestdata data;
  // SimTester modem (SerialAT, 2);
  // modem.simTest(data, 10);
  // test is end ---------------------------------------
  int resetModemPin = 2;
  client = new simTcpClientSocket(SerialAT, resetModemPin);
  client->SimStart(6000, initData.pinCode, 10, false, initData.baudRate);
  client->ModemConnectToGPRS(initData.apn, initData.gprsUser, initData.gprsPass, 10);
}

void loop()
{
  if (!client->connected()) {
    client->connect(host, port);
  }

  client->write(TXbuf, sizeof(TXbuf) - 1);
  delay(1000);

  int size = client->read((uint8_t*)RXbuf, 255);
  if(size) {
    SerialMon.print(RXbuf);
  }
}