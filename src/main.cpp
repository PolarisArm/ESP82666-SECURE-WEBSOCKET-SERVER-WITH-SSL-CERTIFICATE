/*


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
*/


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <WiFiClientSecureBearSSL.h>  // Use the BearSSL client for CA verification

WebSocketsClient socket;

const char *ssid = "Home";
const char *password = "13227296";

// Your CA certificate
const char *ca_cert =  \
"-----BEGIN CERTIFICATE-----\n" \
"MIIGFDCCBPygAwIBAgIQDBq8FSRoabvYT5GzRODDdDANBgkqhkiG9w0BAQsFADA8\n" \
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRwwGgYDVQQDExNBbWF6b24g\n" \
"UlNBIDIwNDggTTAzMB4XDTIzMTIwNDAwMDAwMFoXDTI1MDEwMTIzNTk1OVowFTET\n" \
"MBEGA1UEAxMKZ2xpdGNoLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\n" \
"ggEBALYzCwAH2/5BRidBMK7s0v3/Gmz3YXkEZgYBBB61MOn9ezzPCKhKFrqCKbF8\n" \
"D3Neam0F2tfuBWaisI0FxjDWaN6v6KZcM2C9zRnlp+yEz9g0zbPlHNup8cXl7jvI\n" \
"RghrPvKhtuTvrOjT/s3eyACA1DRGvj4ORkM3nISppmOm7IvWAhhl3ANmC/0HJIsU\n" \
"nBCkNP5zU6LzkwYDg1znWGNB7zP71YM/iwjm6H1bxA+nFjIGqqzuCyKNz6eCf2F5\n" \
"eN5W763dsWIYrRYBqywIxAho0C046gRweXN8dIFcHpT7Rj3qxYx5MWzq6Ase+uo4\n" \
"ccSO6APwc6LEFQE8BwCNs17jSysCAwEAAaOCAzcwggMzMB8GA1UdIwQYMBaAFFXZ\n" \
"GF/SHMwB4Vi0vqvZVUIB1y4CMB0GA1UdDgQWBBRaI2MlNUJpDDvkY27c99+ck9Up\n" \
"ejBpBgNVHREEYjBgggpnbGl0Y2guY29tggoqLmdvbWl4Lm1lgglnb21peC5jb22C\n" \
"CGdvbWl4Lm1lgglnbGl0Y2gubWWCDCouZ2xpdGNoLmNvbYILKi5nb21peC5jb22C\n" \
"CyouZ2xpdGNoLm1lMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA4GA1UdDwEB/wQEAwIF\n" \
"oDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwOwYDVR0fBDQwMjAwoC6g\n" \
"LIYqaHR0cDovL2NybC5yMm0wMy5hbWF6b250cnVzdC5jb20vcjJtMDMuY3JsMHUG\n" \
"CCsGAQUFBwEBBGkwZzAtBggrBgEFBQcwAYYhaHR0cDovL29jc3AucjJtMDMuYW1h\n" \
"em9udHJ1c3QuY29tMDYGCCsGAQUFBzAChipodHRwOi8vY3J0LnIybTAzLmFtYXpv\n" \
"bnRydXN0LmNvbS9yMm0wMy5jZXIwDAYDVR0TAQH/BAIwADCCAX4GCisGAQQB1nkC\n" \
"BAIEggFuBIIBagFoAHcAzxFW7tUufK/zh1vZaS6b6RpxZ0qwF+ysAdJbd87MOwgA\n" \
"AAGMNFHbDAAABAMASDBGAiEAtyfDzy2MuUcRRIFt5hSVqnGKFK8Ogbr9/N8KgeKN\n" \
"aUoCIQCI/RYdVbGrpgqHUagQ036rXGyR61TW1dajzRIBVrfV+QB1AD8XS0/XIkdY\n" \
"lB1lHIS+DRLtkDd/H4Vq68G/KIXs+GRuAAABjDRR2vIAAAQDAEYwRAIgNKBMqbo8\n" \
"bwCvWU2za8c85sBN6oAI8wxcb9A+YLlSTx0CIBvc77yXpKilbCnlX0iV7sjrwgAw\n" \
"/74BP3wJFEyFnGB3AHYAfVkeEuF4KnscYWd8Xv340IdcFKBOlZ65Ay/ZDowuebgA\n" \
"AAGMNFHa3gAABAMARzBFAiEAvleZB5BC1uq20AuO5aoKJNwYqvmOorLR1xwK1K1R\n" \
"cyICICt5jxuFL8RkhnoKmKOGyrxD5O2N65jOBq3/6vkGZoFyMA0GCSqGSIb3DQEB\n" \
"CwUAA4IBAQBL4BiNt9oPrnVug0zzXdnmWcCbxmF569r9W6YXwchx9bJvYgMt8Via\n" \
"zhVFmtYI9ikZ21QTEsyD4GLYwyyw979vvzyVDnlip0iGuSRxuqZ6ffKzS7K4P7hF\n" \
"Cs4bKCD52/BlRQHNK5WH51rH4/J30IIwS6xv+clbk5tc/nxvDUzdsbWWMXw9OK8i\n" \
"SXHwQ8r/lRRgAVgyXzBr1dtYCfnj6pDWIzM9XaasXRLW8djn47VgY0VSS5q6OGt9\n" \
"Q0wplTyodT2acd177v6HCDOoGC1+jBVRltNEo5RqFhsl37tT5J7cPLb7H10mAEKv\n" \
"d4HIyo5AomFU4Su+1ALE6tYQ+lGbO7iD\n" \
"-----END CERTIFICATE-----\n";

int SWITCH_STATE = 0;

unsigned long interval = 1000;
unsigned long previous, current, previous_two = 0;

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

  if(sensor == "SDF")
  {
    SWITCH_STATE = status.toInt();
  }

  Serial.print("Status: ");
  Serial.println(status);
  Serial.print("Sensor: ");
  Serial.println(sensor);
}

void connectToWebSocket() {
  // Secure connection with CA certificate
  socket.beginSslWithCA("websockettestesp32.glitch.me", 443, "/", ca_cert, nullptr);
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

  connectToWebSocket();
  pinMode(LED_BUILTIN,OUTPUT);
}

void loop() {
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
