

#include <Arduino.h>

#include <HTTPClient.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "WiFi.h"

class Wifi_Google
{

public:
    Wifi_Google(const char *ssid, const char *password, const char *serverName);

    void setVariablesPrt(float* voltsSensorVal, uint8_t* percent, int* counter, bool* pumpOn);

    void initWifi();
    void sendToGoogleSheet();


private:
    const char *ssid = nullptr;
    const char *password = nullptr;
    const char *serverName = nullptr;
    unsigned long timeRead = 0;

    float* voltsSensorVal = nullptr;
    uint8_t* percent = nullptr;
    int* counter = nullptr;
    bool* pumpOn = nullptr;
};
