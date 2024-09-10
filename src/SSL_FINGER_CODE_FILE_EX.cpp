#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <WiFiClientSecure.h>

WebSocketsClient socket;

const char *ssid = "Home";
const char *password = "13227296";

boolean connected = false;
int SWITCH_STATE = 0;

unsigned long interval = 1000;
unsigned long previous, current, previous_two = 0;

const uint8_t ssl_fingerprint[20] = {
  0x57, 0x4F, 0x13, 0x8A, 0x33, 0x42, 0x32, 0x7C, 
  0xF7, 0xC9, 0xC5, 0x1F, 0xDF, 0xC1, 0x35, 0x65, 
  0xF0, 0xE9, 0x70, 0xEE
};


String generateRandomDhtData()
{
  int temperature1 = random(10, 70);
  float temperature2 = random(1, 350) / 10.0;
  int humidity = random(70, 90);
  int sensor1 = random(210, 220);
  float value1 = random(20, 30) / 10.0;
  int sensor2 = random(210, 220);
  float value2 = random(20, 30) / 10.0;
  int sensor3 = random(210, 220);
  float value3 = random(20, 30) / 10.0;

  String sensorData = String(temperature1) + "," +
                      String(temperature2, 1) + "," +
                      String(humidity) + "," +
                      String(sensor1) + "," +
                      String(value1, 1) + "," +
                      String(sensor2) + "," +
                      String(value2, 1) + "," +
                      String(sensor3) + "," +
                      String(value3, 1);

  return sensorData;
}

String mpu()
{
  int mpuD = random(0, 10);
  String sensorData = String(mpuD);
  return sensorData;
}



String parseData(String data, int index) {
  int separatorIndex = data.indexOf(':');
  if (separatorIndex == -1) return "";
  if (index == 1) return data.substring(0, separatorIndex);
  else if (index == 2) return data.substring(separatorIndex + 1);
  return "";
}

void handleMessage(WStype_t type, uint8_t *payload, size_t length) {
  String data = (char *)payload;
  String status = parseData(data, 1);
  String sensor = parseData(data, 2);

  Serial.print("Status: ");
  Serial.println(status);
  Serial.print("Sensor: ");
  Serial.println(sensor);

  if (sensor == "SDF")
  {
    SWITCH_STATE = status.toInt();
  }


}




void connectToWebSocket() {
  socket.beginSSL("websockettestesp32.glitch.me", 443, "/", ssl_fingerprint);
  socket.onEvent(handleMessage);
  socket.setReconnectInterval(10000); // Auto-reconnect after 10 seconds
  Serial.println("Connecting to WebSocket server...");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }

  pinMode(LED_BUILTIN,OUTPUT);

  connectToWebSocket();
}

void loop() {
  socket.loop();

  current = millis();

   
    socket.loop();

    if (current - previous >= interval) {
        String sensorData = generateRandomDhtData();
        socket.sendTXT(((sensorData) + ":SENSORDATA").c_str());
        previous = current;
    }
    if (current - previous_two >= 100) {
        String sensorData = mpu();
        socket.sendTXT((sensorData + ":VIB").c_str());
        previous_two = current;
    }

    digitalWrite(LED_BUILTIN,SWITCH_STATE);
}
