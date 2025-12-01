/*
   ESP32 PUBLISHER - TINY IOT (VÍA PLAYIT.GG)
   Host: manufacturer-phases.gl.at.ply.gg
   Port: 33206
*/
#include <WiFi.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- TUS DATOS DE PLAYIT (Copiados de tu imagen) ---
const char* host = "manufacturer-phases.gl.at.ply.gg";
const int port = 33206; 

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client; 

    Serial.print("\nConectando a Playit...");
    
    if (client.connect(host, port)) {
      Serial.println("Conectado!");

      // --- GENERAR DATOS ---
      float temperatura = 20.0 + (random(0, 100) / 10.0);
      
      // --- PROTOCOLO PROPIO (Tiny MQTT) ---
      // Formato: "TOPIC VALOR"
      String mensaje = "sensor/sala/temp " + String(temperatura, 2);
      
      // CAMBIO 1: Agregamos "\n" para que el servidor sepa dónde termina el mensaje
      client.print(mensaje + "\n"); 
      
      // CAMBIO 2: Forzar envío inmediato
      client.flush(); 
      
      Serial.println("[TX] Enviado: " + mensaje);

      // --- LEER RESPUESTA DEL GATEWAY ---
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 2000) {
          Serial.println(">> (Sin respuesta inmediata)");
          break; 
        }
      }

      while(client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(">> [RX] Respuesta: " + line);
      }

      // CAMBIO 3: Pequeña espera técnica antes de cerrar
      delay(100);
      client.stop();
      
    } else {
      Serial.println("Error: No se pudo conectar al túnel.");
    }
  }

  // IMPORTANTE: Esperar 5 segundos antes de repetir el ciclo
  delay(5000); 
}