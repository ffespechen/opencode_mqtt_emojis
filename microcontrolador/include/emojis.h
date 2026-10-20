// ============================================================
//  Declaración de las funciones de dibujo de emojis.
// ============================================================
#pragma once

#include <TFT_eSPI.h>

// Dibuja el emoji indicado (por su nombre, p. ej. "feliz", "calabaza")
// centrado en (cx, cy) con un factor de escala `escala` (1.0 = tamaño normal).
void dibujarEmoji(TFT_eSPI& tft, const char* nombre, int16_t cx, int16_t cy, float escala = 1.0f);
