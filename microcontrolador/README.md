# Microcontrolador — Emojis MQTT

Firmware para **ESP32** (framework **Arduino**, proyecto **PlatformIO**) que se
suscribe al canal MQTT `ffespechen/mqtt/emojis/` del broker público
`broker.emqx.io` y reacciona a los mensajes que publica la aplicación web.

## Comportamiento

Al recibir un mensaje `{"emoji": "...", "estado": "..."}`:

1. Enciende el **LED** del color indicado en `estado`.
2. Muestra en la pantalla una **animación de 3 s** del emoji (efecto de rebote).
3. Mantiene el emoji **estático 5 s**.
4. Apaga el LED y muestra el mensaje **"Esperando nuevo msj"**.

Los valores posibles de `estado` (y por tanto los LEDs) son:
`rojo`, `amarillo`, `verde`, `azul`, `naranja`.

## Componentes requeridos

| Cantidad | Componente | Especificación / notas |
|:---:|---|---|
| 1 | Placa de desarrollo **ESP32** | DevKit V1 / NodeMCU-32S / ESP32-DevKitC (módulo con WiFi) |
| 1 | Pantalla redonda **GC9A01** | 1.28", 240×240 px, interfaz SPI de 4 hilos, lógica **3.3 V** |
| 5 | LEDs de 5 mm | rojo, amarillo, verde, azul y naranja (difusos, recomendados) |
| 5 | Resistencias de ¼ W | 150 Ω (rojo/amarillo/verde/naranja) y 100 Ω (azul) — ver tabla |
| 1 | Protoboard | 400 puntos (o suficiente para el montaje) |
| — | Cables dupont | macho-macho y macho-hembra |
| 1 | Cable USB | micro-USB o USB-C según la placa (alimentación + programación) |

Opcional: fuente de 5 V estable, si la alimentación por USB resultara insuficiente.

## Diagrama de conexiones

```text
   ESP32 (DevKit V1)                       Pantalla GC9A01 (SPI)

   3V3           ──────────────────────────►  VCC
   GND           ──────────────────────────►  GND
   GPIO18 (SCLK) ──────────────────────────►  SCL
   GPIO23 (MOSI) ──────────────────────────►  SDA
   GPIO19 (MISO) ──────────────────────────►  MISO
   GPIO15 (CS)   ──────────────────────────►  CS
   GPIO2  (DC)   ──────────────────────────►  DC
   GPIO4  (RST)  ──────────────────────────►  RST


   ESP32 (DevKit V1)                       LEDs

   GPIO25 ──[150 Ω]──► LED rojo      ──► GND
   GPIO26 ──[150 Ω]──► LED amarillo  ──► GND
   GPIO27 ──[150 Ω]──► LED verde     ──► GND
   GPIO14 ──[100 Ω]──► LED azul      ──► GND
   GPIO12 ──[150 Ω]──► LED naranja   ──► GND
```

> En los LEDs, el **ánodo** (pata larga) va al GPIO a través de la resistencia y
> el **cátodo** (pata corta) va a **GND**.

## Tabla de conexiones

### Pantalla GC9A01 (SPI)

| Señal de la pantalla | GPIO ESP32 |
|----------------------|:----------:|
| SCL (SCLK)           | 18         |
| SDA (MOSI)           | 23         |
| MISO                 | 19         |
| CS                   | 15         |
| DC (RS)              | 2          |
| RST                  | 4          |
| VCC                  | 3.3 V      |
| GND                  | GND        |
| BL (retroiluminación)| 3.3 V (o según el módulo) |

> Estos pines siguen el setup oficial `Setup46_GC9A01_ESP32` de TFT_eSPI.
> Si tu módulo usa otros pines (o GPIO 2/15 te causan problemas de arranque),
> modifícalos en `platformio.ini` (bloque `build_flags`).

### LEDs (ánodo por GPIO a través de resistencia, cátodo a GND)

| Color   | GPIO ESP32 | Vf típico | Resistencia sugerida (≈8 mA) |
|---------|:----------:|:---------:|:----------------------------:|
| rojo    | 25         | ~2.0 V    | 150 Ω                        |
| amarillo| 26         | ~2.1 V    | 150 Ω                        |
| verde   | 27         | ~2.1 V    | 150 Ω                        |
| azul    | 14         | ~3.2 V    | 100 Ω (ver nota)             |
| naranja | 12         | ~2.1 V    | 150 Ω                        |

Los pines de los LEDs se configuran en `include/config.h`.

## Cuidados y consideraciones

- **Niveles lógicos a 3.3 V**: la pantalla GC9A01 y el ESP32 trabajan a 3.3 V.
  **No conectar** las señales SPI ni los pines de control a 5 V, o dañarás el
  display y/o el microcontrolador.
- **Alimentación de la pantalla**: alimentar el módulo con **3.3 V**. Si el
  módulo trae regulador integrado puede aceptar 5 V, pero verifica su hoja de
  datos antes de hacerlo.
- **Consumo**: la retroiluminación de la pantalla suele consumir ~50–80 mA.
  Alimentar el conjunto por el **USB de la placa ESP32** es suficiente para este
  proyecto (pantalla + 5 LEDs + ESP32 ≈ 100–150 mA en total).
- **Cálculo de resistencias de LEDs**: `R = (3.3 V − Vf) / I`. Con 150 Ω se
  obtienen ~8 mA en los LEDs de Vf ≈ 2 V; **220 Ω es un valor universal seguro**
  (~5 mA). No bajes de ~100 Ω sin recalcular: cada GPIO no debería superar
  ~12 mA (máximo absoluto 40 mA).
- **LED azul (y verdes/blancos de alto brillo)**: tienen Vf de 3.0–3.4 V,
  cercano o superior a 3.3 V, por lo que alimentados directamente del GPIO
  pueden quedar **tenues o no encender**. Alternativas: usar un LED azul de Vf
  bajo, o manejarlo con un transistor (p. ej. 2N2222) desde 5 V.
- **Pines de arranque (strapping)**: GPIO 2 (DC) y GPIO 15 (CS) son pines de
  arranque. El setup oficial de TFT_eSPI los usa sin problema, pero si el ESP32
  no arranca, reubica DC/CS a pines libres (p. ej. DC→5, CS→4) y actualiza
  `platformio.ini`.
- **GND común**: conecta entre sí el GND del ESP32, de la pantalla y de los LEDs.
- **Cablea sin energía**: realiza las conexiones con la placa **desconectada**
  para evitar cortocircuitos o daños por conexión en caliente.
- **No excedas 3.3 V** en ningún pin de la pantalla ni en los GPIO del ESP32.

## Configuración

1. Abre `include/config.h` y edita las credenciales WiFi:

   ```cpp
   #define WIFI_SSID      "TU_NOMBRE_DE_RED"
   #define WIFI_PASSWORD  "TU_CONTRASENA"
   ```

2. (Opcional) Ajusta pines, broker, topic y tiempos de la secuencia en el
   mismo archivo.

## Compilar y flashear

```bash
cd microcontrolador
pio run               # compila
pio run -t upload     # compila y flashea (con el ESP32 conectado)
pio device monitor    # monitor serie (115200 baud)
```

> Si el comando `pio` no está en tu PATH, instala PlatformIO Core
> (https://docs.platformio.org/en/latest/core/installation.html).

## Estructura del proyecto

```
microcontrolador/
├── platformio.ini        # Configuración de PlatformIO (placa, librerías,
│                         #   y configuración de pantalla TFT_eSPI vía -D)
├── include/
│   ├── config.h          # WiFi, broker, topic, pines de LEDs y tiempos
│   └── emojis.h          # Declaración de dibujarEmoji()
├── src/
│   ├── main.cpp          # setup/loop, WiFi, MQTT, LEDs y máquina de estados
│   └── emojis.cpp        # Dibujo vectorial de los 30 emojis
├── lib/                  # Librerías locales (vacío)
└── test/                 # Pruebas unitarias (vacío)
```

## Dependencias (vía `lib_deps`)

| Librería      | Uso                              |
|---------------|----------------------------------|
| `TFT_eSPI`    | Controlador de la pantalla GC9A01|
| `PubSubClient`| Cliente MQTT                     |
| `ArduinoJson` | Parseo del JSON del mensaje      |

## Notas de diseño

- La configuración de la pantalla se pasa mediante flags `-D` en
  `platformio.ini` (`USER_SETUP_LOADED`, `GC9A01_DRIVER`, pines, fuentes y
  frecuencias). Así **tanto el sketch como la librería TFT_eSPI** compilan con
  el driver GC9A01 (editar `User_Setup.h` dentro de `.pio/` no funciona porque
  esa carpeta se regenera).
- Los emojis se dibujan como **gráficos vectoriales** (círculos, arcos, elipses
  y triángulos) para no necesitar assets externos. Cada uno se escala en torno
  a su centro, lo que permite la animación de rebote.
- La máquina de estados (`ESPERANDO → ANIMANDO → ESTATICO → ESPERANDO`) se
  controla con `millis()`, sin bloquear el bucle principal.
