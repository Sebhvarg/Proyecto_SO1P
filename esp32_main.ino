#include <WiFi.h>

// Lightweight custom protocol over TCP to your gateway
// Each line: "publisher<ID>/<metric> <value>" (matches C gateway/broker)
// Example: "publisher1/temperature 26°C" then "publisher1/humidity 55%"

// WiFi config (Wokwi-GUEST in simulator or your WiFi)
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// Gateway tunnel endpoint (use ngrok or public IP:port to your gateway)
// Set these to the TCP address that forwards to your local gateway port (1884 for gateway 1)
String GATEWAY_HOST = "0.tcp.sa.ngrok.io";  // placeholder; replace after creating tunnel
uint16_t GATEWAY_PORT = 19343;          // placeholder; replace after creating tunnel

// Publisher identity and metrics
int publisherId = 1; // adjust per instance

WiFiClient client;

// FreeRTOS tasks
TaskHandle_t tempTaskHandle = nullptr;
TaskHandle_t humidTaskHandle = nullptr;
TaskHandle_t sendTaskHandle = nullptr;

// Shared sensor data
volatile int currentTemp = 25;
volatile int currentHumid = 50;

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado a WiFi, IP ");
  Serial.println(WiFi.localIP());
}

bool connectGateway() {
  Serial.print("Conectando a Gateway:  ");
  Serial.print(GATEWAY_HOST);
  Serial.print(":");
  Serial.println(GATEWAY_PORT);
  if (client.connected()) return true;
  if (client.connect(GATEWAY_HOST.c_str(), GATEWAY_PORT)) {
    Serial.println("Gateway: conectado");
    return true;
  } else {
    Serial.println("Gateway: no conectado");
    return false;
  }
}

// Simulate temperature sensor
void tempTask(void* pv) {
  for (;;) {
    int base = 22 + (millis() / 60000) % 6; // drift
    currentTemp = base + (esp_random() % 4); // 22..31-ish
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

// Simulate humidity sensor
void humidTask(void* pv) {
  for (;;) {
    int base = 45 + (millis() / 60000) % 10;
    currentHumid = base + (esp_random() % 6); // 45..65-ish
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// Send both metrics periodically in text mode
void sendTask(void* pv) {
  char line[128];
  for (;;) {
    if (!client.connected()) {
      connectGateway();
    }
    if (client.connected()) {
      // temperature
      snprintf(line, sizeof(line), "publisher%d/temperature %d°C\n", publisherId, currentTemp);
      client.print(line);
      Serial.print("TX: "); Serial.print(line);
      vTaskDelay(pdMS_TO_TICKS(1000));

      // humidity
      snprintf(line, sizeof(line), "publisher%d/humidity %d%%\n", publisherId, currentHumid);
      client.print(line);
      Serial.print("TX: "); Serial.print(line);
    }
    vTaskDelay(pdMS_TO_TICKS(4000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  connectWifi();

  // Start tasks
  xTaskCreatePinnedToCore(tempTask,   "tempTask",  2048, nullptr, 1, &tempTaskHandle, 1);
  xTaskCreatePinnedToCore(humidTask,  "humidTask", 2048, nullptr, 1, &humidTaskHandle, 1);
  xTaskCreatePinnedToCore(sendTask,   "sendTask",  4096, nullptr, 1, &sendTaskHandle, 0);
}

void loop() {
  // no-op; work done in FreeRTOS tasks
  vTaskDelay(pdMS_TO_TICKS(1000));
}
