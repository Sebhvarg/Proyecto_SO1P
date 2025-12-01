#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
// URL de ngrok proporcionada por el usuario (Cambiado a HTTP)
const char* serverName = "http://casuistic-obesely-sharice.ngrok-free.dev";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client; // Usar cliente normal, no seguro
    HTTPClient http;
    
    if (http.begin(client, serverName)) {  // Iniciar conexion
      http.addHeader("Content-Type", "text/plain");
      // Importante: Ngrok pide un header extra para saltar la pagina de advertencia en cuentas gratuitas
      http.addHeader("ngrok-skip-browser-warning", "true");

      // Generar datos aleatorios
      int value = random(0, 100);
      String postData = "sensor1 " + String(value);
      
      int httpResponseCode = http.POST(postData);
      
      Serial.print("Sending: ");
      Serial.println(postData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(response);
      } else {
        Serial.print("Error code: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Unable to connect");
    }
  }
  delay(5000);
}
