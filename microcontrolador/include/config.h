// ============================================================
//  Configuración del proyecto "Emojis MQTT" (microcontrolador)
//  Edita los valores de esta sección según tu hardware y red.
// ============================================================
#pragma once

// ---------- Red WiFi ----------
#define WIFI_SSID      "TU_NOMBRE_DE_RED"
#define WIFI_PASSWORD  "TU_CONTRASENA"

// ---------- Broker MQTT ----------
#define MQTT_SERVER    "broker.emqx.io"
#define MQTT_PORT      1883
#define MQTT_TOPIC     "ffespechen/mqtt/emojis/"

// ---------- Pines de los LEDs (un LED por cada color/estado) ----------
// Los pines de la pantalla se definen en platformio.ini (build_flags)
// (TFT: MISO=19, MOSI=23, SCLK=18, CS=15, DC=2, RST=4).
#define PIN_LED_ROJO     25
#define PIN_LED_AMARILLO 26
#define PIN_LED_VERDE    27
#define PIN_LED_AZUL     14
#define PIN_LED_NARANJA  12

// ---------- Temporización de la secuencia en pantalla (ms) ----------
#define DURACION_ANIMACION_MS  3000   // animación del emoji (rebote)
#define DURACION_ESTATICO_MS   5000   // emoji estático
