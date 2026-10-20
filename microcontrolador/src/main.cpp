// ============================================================
//  Emojis MQTT - Microcontrolador (ESP32 + GC9A01 + LEDs)
//
//  - Se conecta a la red WiFi y al broker MQTT broker.emqx.io.
//  - Se suscribe al topic "ffespechen/mqtt/emojis/".
//  - Al recibir {"emoji": "...", "estado": "..."}:
//      1) enciende el LED del color "estado";
//      2) muestra una animación de 3 s del emoji;
//      3) mantiene el emoji estático 5 s;
//      4) apaga el LED y muestra "Esperando nuevo msj".
// ============================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

#include "config.h"
#include "emojis.h"

// ---------- objetos globales ----------
TFT_eSPI tft;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// ---------- máquina de estados de la pantalla ----------
enum class EstadoPantalla { ESPERANDO, ANIMANDO, ESTATICO };

EstadoPantalla estado = EstadoPantalla::ESPERANDO;
char emojiActual[32] = "";
char estadoColorActual[16] = "";
unsigned long marcaTiempo = 0;
unsigned long ultimoFrame = 0;

// ---------- LEDs ----------
const int pinesLed[] = {
    PIN_LED_ROJO, PIN_LED_AMARILLO, PIN_LED_VERDE, PIN_LED_AZUL, PIN_LED_NARANJA};

void apagarLeds() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(pinesLed[i], LOW);
  }
}

int pinPorEstado(const char* color) {
  if (strcmp(color, "rojo") == 0) return PIN_LED_ROJO;
  if (strcmp(color, "amarillo") == 0) return PIN_LED_AMARILLO;
  if (strcmp(color, "verde") == 0) return PIN_LED_VERDE;
  if (strcmp(color, "azul") == 0) return PIN_LED_AZUL;
  if (strcmp(color, "naranja") == 0) return PIN_LED_NARANJA;
  return -1;
}

void encenderLed(const char* color) {
  apagarLeds();
  int pin = pinPorEstado(color);
  if (pin >= 0) {
    digitalWrite(pin, HIGH);
  }
}

// ---------- pantalla ----------
void mostrarEsperando() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Esperando", 120, 100, 4);
  tft.drawString("nuevo msj", 120, 140, 4);
  apagarLeds();
}

// ---------- animación: función de easing "rebote" ----------
float easeOutBounce(float t) {
  const float n1 = 7.5625f;
  const float d1 = 2.75f;
  if (t < 1.0f / d1) {
    return n1 * t * t;
  } else if (t < 2.0f / d1) {
    t -= 1.5f / d1;
    return n1 * t * t + 0.75f;
  } else if (t < 2.5f / d1) {
    t -= 2.25f / d1;
    return n1 * t * t + 0.9375f;
  } else {
    t -= 2.625f / d1;
    return n1 * t * t + 0.984375f;
  }
}

// ---------- WiFi ----------
void conectarWifi() {
  Serial.printf("Conectando a WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi conectado. IP: " + WiFi.localIP().toString());
}

// ---------- MQTT ----------
void reconectarMqtt() {
  if (mqtt.connected()) {
    return;
  }
  String clientId = "esp32_emojis_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.print("Conectando a MQTT... ");
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("conectado");
    mqtt.subscribe(MQTT_TOPIC);
    Serial.printf("Suscrito a: %s\n", MQTT_TOPIC);
  } else {
    Serial.printf("fallo (rc=%d), reintentando en 5 s\n", mqtt.state());
  }
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  char buf[256];
  unsigned int n = (length < sizeof(buf) - 1) ? length : sizeof(buf) - 1;
  memcpy(buf, payload, n);
  buf[n] = '\0';

  JsonDocument doc;
  if (deserializeJson(doc, buf)) {
    Serial.println("[MQTT] JSON invalido");
    return;
  }

  const char* emoji = doc["emoji"] | "";
  const char* color = doc["estado"] | "";
  if (strlen(emoji) == 0 || strlen(color) == 0) {
    return;
  }

  strncpy(emojiActual, emoji, sizeof(emojiActual) - 1);
  emojiActual[sizeof(emojiActual) - 1] = '\0';
  strncpy(estadoColorActual, color, sizeof(estadoColorActual) - 1);
  estadoColorActual[sizeof(estadoColorActual) - 1] = '\0';

  encenderLed(estadoColorActual);

  estado = EstadoPantalla::ANIMANDO;
  marcaTiempo = millis();
  ultimoFrame = 0;

  Serial.printf("[MQTT] emoji=%s estado=%s\n", emojiActual, estadoColorActual);
}

// ---------- setup / loop ----------
void setup() {
  Serial.begin(115200);

  // LEDs
  for (int i = 0; i < 5; i++) {
    pinMode(pinesLed[i], OUTPUT);
  }
  apagarLeds();

  // Pantalla
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  mostrarEsperando();

  // WiFi + MQTT
  conectarWifi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callbackMqtt);
  mqtt.setBufferSize(256);
  reconectarMqtt();
}

void loop() {
  // Mantener la conexión MQTT activa
  if (!mqtt.connected()) {
    static unsigned long ultimoReintento = 0;
    if (millis() - ultimoReintento > 5000) {
      ultimoReintento = millis();
      reconectarMqtt();
    }
  }
  mqtt.loop();

  switch (estado) {
    case EstadoPantalla::ANIMANDO: {
      unsigned long t = millis() - marcaTiempo;
      if (t >= DURACION_ANIMACION_MS) {
        // fin de la animación -> emoji estático
        estado = EstadoPantalla::ESTATICO;
        marcaTiempo = millis();
        tft.fillScreen(TFT_BLACK);
        dibujarEmoji(tft, emojiActual, 120, 120, 1.0f);
        break;
      }
      if (millis() - ultimoFrame >= 33) {
        ultimoFrame = millis();
        float p = (float)t / DURACION_ANIMACION_MS;
        float escala = 0.3f + 0.7f * easeOutBounce(p);
        tft.fillScreen(TFT_BLACK);
        dibujarEmoji(tft, emojiActual, 120, 120, escala);
      }
      break;
    }

    case EstadoPantalla::ESTATICO: {
      if (millis() - marcaTiempo >= DURACION_ESTATICO_MS) {
        estado = EstadoPantalla::ESPERANDO;
        mostrarEsperando();
      }
      break;
    }

    case EstadoPantalla::ESPERANDO:
    default:
      break;
  }
}
