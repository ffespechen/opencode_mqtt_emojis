import mqtt from 'mqtt';

// Broker público EMQX (WebSocket seguro). El puerto 8084 corresponde a WSS.
const BROKER_URL = 'wss://broker.emqx.io:8084/mqtt';

// Canal al que se publican los emojis.
export const MQTT_TOPIC = 'ffespechen/mqtt/emojis/';

let client = null;

/**
 * Conecta al broker MQTT y notifica los cambios de estado de conexión
 * a través del callback `onStatusChange(connected: boolean)`.
 */
export function connectMqtt(onStatusChange) {
  client = mqtt.connect(BROKER_URL, {
    clientId: `emojis_mqtt_web_${Math.random().toString(16).slice(2, 10)}`,
    keepalive: 60,
    clean: true,
    reconnectPeriod: 3000,
  });

  client.on('connect', () => onStatusChange(true));
  client.on('reconnect', () => onStatusChange(false));
  client.on('offline', () => onStatusChange(false));
  client.on('close', () => onStatusChange(false));
  client.on('error', () => onStatusChange(false));
}

/**
 * Publica un mensaje JSON con las claves `emoji` y `estado` en el topic.
 * Devuelve `true` si la publicación fue posible (cliente conectado).
 */
export function publishEmoji(emoji, estado) {
  if (!client || !client.connected) {
    return false;
  }

  client.publish(MQTT_TOPIC, JSON.stringify({ emoji, estado }));
  return true;
}
