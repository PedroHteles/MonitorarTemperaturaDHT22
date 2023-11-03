#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

#define DHTPIN1 23
#define DHTPIN2 21
#define DHTPIN3 25
#define DHTPIN4 26
#define DHTPIN5 27
#define DHTPIN6 33
#define DHTTYPE DHT22

DHT dht[] = {DHT(DHTPIN1, DHTTYPE), DHT(DHTPIN2, DHTTYPE), DHT(DHTPIN3, DHTTYPE), DHT(DHTPIN4, DHTTYPE), DHT(DHTPIN5, DHTTYPE), DHT(DHTPIN6, DHTTYPE)};
int numSensors = sizeof(dht) / sizeof(dht[0]);

const char *ssid = "corporativo";
const char *password = "arcom2022";

WebServer server(80);

void setup()
{
    Serial.begin(115200);

    for (int i = 0; i < numSensors; i++)
    {
        dht[i].begin();
        setupSensorRoute(i + 1); // Create a route for each sensor
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.print("Connected to: ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/dados", HTTP_GET, handleAllSensorData);

    server.begin();
}

void loop()
{
    server.handleClient();
}

void setupSensorRoute(int sensorIndex)
{
    server.on("/dados" + String(sensorIndex), HTTP_GET, [sensorIndex]()
              { handleSensorData(sensorIndex); });
}

void handleSensorData(int sensorIndex)
{
    int sensorPin = DHTPIN1 + sensorIndex - 1;
    float h = dht[sensorIndex - 1].readHumidity();
    float t = dht[sensorIndex - 1].readTemperature();

    if (isnan(h) || isnan(t))
    {
        Serial.println("Sensor " + String(sensorIndex) + " reading failed");
        server.send(500, "text/plain", "Sensor reading failed");
        return;
    }

    String json = "{\"id\":" + String(sensorIndex) + ",\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}";
    server.send(200, "application/json", json);
}

void handleAllSensorData()
{
    String json = "[";

    for (int i = 0; i < numSensors; i++)
    {
        float h = dht[i].readHumidity();
        float t = dht[i].readTemperature();
        String sensorStatus = !isnan(h) && !isnan(t) ? ("{\"id\":" + String(i + 1) + ",\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}") : "\"Error in sensor\"";

        json += sensorStatus;

        if (i < numSensors - 1)
        {
            json += ",";
        }
    }

    json += "]";

    server.send(200, "application/json", json);
}
