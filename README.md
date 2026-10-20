# Emojis MQTT

Monorepo que implementa un sistema de publicación de emojis sobre MQTT.

## Estructura

```
opencode_mqtt_emojis/
├── web/                 # Cliente MQTT (React + JavaScript + Vite)
└── microcontrolador/    # Firmware ESP32 (PlatformIO + Arduino) + GC9A01 + LEDs
```

## Broker y canal

- **Broker:** `broker.emqx.io` (broker público EMQX).
- **Canal (topic):** `ffespechen/mqtt/emojis/`
- **Web:** WebSocket seguro `wss://broker.emqx.io:8084/mqtt`
- **ESP32:** MQTT/TCP `broker.emqx.io:1883`

## Formato del mensaje

Cada clic publica un mensaje JSON con las claves:

```json
{ "emoji": "feliz", "estado": "rojo" }
```

- `emoji`: nombre del emoji elegido.
- `estado`: uno de `rojo`, `amarillo`, `verde`, `azul`, `naranja`.

## Aplicación web (`/web`)

Cliente MQTT construido con React + JavaScript + Vite.

### Requisitos

- Node.js >= 18

### Instalación y ejecución

```bash
cd web
npm install
npm run dev      # servidor de desarrollo
npm run build    # build de producción (carpeta dist/)
npm run preview  # previsualizar el build
```

### Funcionamiento

- Muestra el título **"Emojis MQTT"** y una cuadrícula de **6 columnas × 5 filas**
  con **30 emojis** (cada botón de al menos 64×64 px).
- Al hacer clic en un emoji, el color `estado` **rota cíclicamente** en el orden:
  `rojo → amarillo → verde → azul → naranja → rojo...`
- Cada clic publica el mensaje JSON en el topic indicado.

### Dependencias principales

- `react` / `react-dom`: interfaz de usuario.
- `mqtt` (MQTT.js): cliente MQTT sobre WebSocket en el navegador.
- `vite` + `@vitejs/plugin-react`: bundler y tooling de desarrollo.

## Microcontrolador (`/microcontrolador`)

Proyecto **PlatformIO** para un **ESP32** (framework Arduino) con pantalla
redonda **GC9A01** (240×240) y 5 LEDs (uno por color).

- Se suscribe al topic MQTT y, al recibir un mensaje:
  1. enciende el LED del color `estado`;
  2. muestra una **animación de 3 s** del emoji;
  3. lo mantiene **estático 5 s**;
  4. muestra **"Esperando nuevo msj"** y apaga el LED.

Compilación (ver `microcontrolador/README.md` para pines, conexiones y
configuración de credenciales WiFi en `include/config.h`):

```bash
cd microcontrolador
pio run               # compila
pio run -t upload     # flashea
```

---

## Registro de tareas

| Requerimiento | Archivos / módulos | Solución |
| --- | --- | --- |
| Monorepo con carpetas `web` y `microcontrolador` | `/web`, `/microcontrolador` | Estructura de dos carpetas independientes. |
| Cliente MQTT (React + JS + Vite) | `/web/package.json`, `/web/vite.config.js`, `/web/index.html`, `/web/src/*` | App Vite + React que conecta al broker por WebSocket y publica en el topic. |
| Título "Emojis MQTT" y cuadrícula 6×5 (30 emojis ≥64×64) | `/web/src/App.jsx`, `/web/src/App.css`, `/web/src/emojis.js` | Cabecera con título y grid CSS de 6 columnas; botones de mínimo 64×64. |
| Publicación JSON `{ emoji, estado }` con ciclo de color | `/web/src/mqtt.js`, `/web/src/App.jsx` | `publishEmoji()` publica el JSON; el color rota con un índice por emoji. |
| Proyecto PlatformIO + Arduino para ESP32 | `/microcontrolador/platformio.ini`, `/microcontrolador/include/*`, `/microcontrolador/src/*` | Entorno `esp32dev` con TFT_eSPI, PubSubClient y ArduinoJson. |
| Pantalla GC9A01 configurada en la librería | `/microcontrolador/platformio.ini` (build_flags) | Flags `-D GC9A01_DRIVER` + pines/fuentes para que la librería TFT_eSPI compile el driver correcto. |
| Suscripción MQTT y encendido del LED por color | `/microcontrolador/src/main.cpp` | `callbackMqtt` parsea el JSON, enciende el LED del `estado` y lanza la secuencia. |
| Animación 3 s + estático 5 s + "Esperando nuevo msj" | `/microcontrolador/src/main.cpp`, `/microcontrolador/src/emojis.cpp` | Máquina de estados con `millis()` y easing de rebote; `dibujarEmoji()` dibuja los 30 emojis en vectorial. |
