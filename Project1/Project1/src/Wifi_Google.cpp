

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
    int status = WiFi.waitForConnectResult(10000);

#ifdef DEBUG_WIFI_GOOGLE

    if (status == WL_CONNECTED)
    {
        Serial.println("Connected!");
    }
    else
    {
        Serial.print("Failed to connect. Status: ");
        Serial.println(status);
    }
    printf("WiFi connected with IP address: Local IP: %s on %s\n", WiFi.localIP().toString(), ssid);
#endif
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
        int httpResponseCode = http.POST(jsonData);
        http.end();

#ifdef DEBUG_WIFI_GOOGLE

        printf("voltsSensorVal: %.3f percent:  %d counter: %d pumpOn %d \n", *voltsSensorVal, *percent, *counter, *pumpOn);

        if (httpResponseCode > 0)
        {
            String response = http.getString();

            printf("Request POST Sent \n");
        }
        else
        {
            printf("Wrong request POST->  %d", httpResponseCode);
        }
#endif
    }
}
