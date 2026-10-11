

#include "Wifi_Google.h"

Wifi_Google::Wifi_Google(const char *ssid, const char *password, const char *serverName)
{
    this->ssid = ssid;
    this->password = password;
    this->serverName = serverName;
    this->timeRead = 0;
}

void Wifi_Google::setVariablesPrt(float *voltsSensorVal, uint8_t *percent, int *counter, bool *pumpOn)
{
    this->voltsSensorVal = voltsSensorVal;
    this->percent = percent;
    this->counter = counter;
    this->pumpOn = pumpOn;
}

void Wifi_Google::initWifi()
{

    if (ssid == nullptr || password == nullptr)
    {
        printf("SSID or Password is null. Cannot connect to WiFi.\n");
        return;
    }

    WiFi.begin(ssid, password);

    int timeout = 10 * 4; // 10 seconds
    while (WiFi.status() != WL_CONNECTED && (timeout-- > 0))
    {
        delay(250);
        printf(".");
    }
    // Serial.println("");

    printf("  ");

    if (WiFi.status() != WL_CONNECTED)
    {
        printf(("Failed to connect\n"));
    }

    printf("WiFi connected with IP address: Local IP: %s\n", WiFi.localIP());
}

void Wifi_Google::sendToGoogleSheet()
{
    if (serverName == nullptr)
    {
        printf("Server name is null. Cannot send data to Google Sheet.\n");
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");

        String jsonData = "{\"volt\":\"" + String(*voltsSensorVal, 3) + "\", \"moist\":\"" + String(*percent) + "\", \"count\":\"" + String(*counter) + "\", \"pumpON\":\"" + String(*pumpOn) + "\"}";

        printf("voltsSensorVal: %.3f percent:  %d counter: %d pumpOn %d \n", *voltsSensorVal, *percent, *counter, *pumpOn);

        int httpResponseCode = http.POST(jsonData);

        if (httpResponseCode > 0)
        {
            String response = http.getString();

            printf("Request POST Sent \n");
        }
        else
        {
            printf("Wrong request POST->  %d", httpResponseCode);
        }

        http.end();
    }

    delay(1000);
}
