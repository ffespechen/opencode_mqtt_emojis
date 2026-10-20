import { useEffect, useRef, useState } from 'react';
import { emojis } from './emojis.js';
import { connectMqtt, publishEmoji } from './mqtt.js';
import './App.css';

// Secuencia de colores. Cada clic sobre un emoji avanza al siguiente
// color de forma cíclica.
const COLORS = ['rojo', 'amarillo', 'verde', 'azul', 'naranja'];

function App() {
  const [connected, setConnected] = useState(false);
  // Guarda el índice del siguiente color para cada emoji (sin re-render).
  const colorIndex = useRef({});

  useEffect(() => {
    connectMqtt(setConnected);
  }, []);

  const handleClick = (emoji) => {
    const idx = colorIndex.current[emoji.nombre] ?? 0;
    const estado = COLORS[idx];
    colorIndex.current[emoji.nombre] = (idx + 1) % COLORS.length;

    publishEmoji(emoji.nombre, estado);
  };

  return (
    <div className="app">
      <header className="header">
        <h1>Emojis MQTT</h1>
        <div className={`status ${connected ? 'connected' : 'disconnected'}`}>
          <span className="status-dot" aria-hidden="true" />
          {connected ? 'Conectado al broker' : 'Desconectado'}
        </div>
      </header>

      <main className="grid">
        {emojis.map((emoji) => (
          <button
            key={emoji.nombre}
            type="button"
            className="emoji-card"
            title={emoji.nombre}
            onClick={() => handleClick(emoji)}
          >
            <span className="emoji-symbol">{emoji.simbolo}</span>
            <span className="emoji-name">{emoji.nombre}</span>
          </button>
        ))}
      </main>
    </div>
  );
}

export default App;
