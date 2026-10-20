// ============================================================
//  Dibujo de los emojis mediante primitivas vectoriales de TFT_eSPI.
//
//  Cada emoji se dibuja centrado en (cx, cy) y escalado por `escala`
//  (útil para la animación de rebote). Las representaciones son
//  simplificaciones vectoriales (círculos, arcos, elipses, triángulos).
// ============================================================

#include "emojis.h"

#include <math.h>
#include <string.h>

namespace {

// ---------- utilidades de escala ----------
inline int16_t X(int16_t cx, float dx, float e) { return cx + (int16_t)(dx * e); }
inline int16_t Y(int16_t cy, float dy, float e) { return cy + (int16_t)(dy * e); }
inline int16_t R(float r, float e) { return (int16_t)(r * e); }

// ---------- colores ----------
constexpr uint16_t C_AMARILLO = 0xFE60;   // amarillo emoji (255, 204, 0)
constexpr uint16_t C_VERDE   = 0x3E20;    // verde dinero aproximado
constexpr uint16_t C_ALIEN   = 0x3E80;    // verde alien aproximado

// ---------- formas pequeñas ----------
void corazonChico(TFT_eSPI& t, int16_t cx, int16_t cy, float e, uint16_t color) {
  int16_t r = R(7, e);
  t.fillCircle(X(cx, -5, e), Y(cy, -4, e), r, color);
  t.fillCircle(X(cx, 5, e),  Y(cy, -4, e), r, color);
  t.fillTriangle(X(cx, -11, e), Y(cy, 1, e), X(cx, 11, e), Y(cy, 1, e), cx, Y(cy, 16, e), color);
}

// Estrella de 5 puntas
void estrella(TFT_eSPI& t, int16_t cx, int16_t cy, float e, int16_t radio, uint16_t color) {
  const int n = 5;
  int16_t ox[n], oy[n], ix[n], iy[n];
  for (int i = 0; i < n; i++) {
    float ao = -PI / 2 + i * 2 * PI / n;
    float ai = ao + PI / n;
    ox[i] = X(cx, cosf(ao) * radio, e);
    oy[i] = Y(cy, sinf(ao) * radio, e);
    ix[i] = X(cx, cosf(ai) * radio * 0.5f, e);
    iy[i] = Y(cy, sinf(ai) * radio * 0.5f, e);
  }
  for (int i = 0; i < n; i++) {
    int j = (i + 1) % n;
    t.fillTriangle(cx, cy, ox[i], oy[i], ix[i], iy[i], color);
    t.fillTriangle(cx, cy, ix[i], iy[i], ox[j], oy[j], color);
  }
}

// ---------- cara base ----------
void caraBase(TFT_eSPI& t, int16_t cx, int16_t cy, float e, uint16_t color) {
  t.fillCircle(cx, cy, R(80, e), color);
}

// ---------- ojos ----------
void ojosPuntos(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(X(cx, -26, e), Y(cy, -20, e), R(9, e), TFT_BLACK);
  t.fillCircle(X(cx, 26, e),  Y(cy, -20, e), R(9, e), TFT_BLACK);
}

void ojosGrandes(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(X(cx, -24, e), Y(cy, -20, e), R(14, e), TFT_BLACK);
  t.fillCircle(X(cx, 24, e),  Y(cy, -20, e), R(14, e), TFT_BLACK);
  t.fillCircle(X(cx, -24, e), Y(cy, -20, e), R(5, e), TFT_WHITE);
  t.fillCircle(X(cx, 24, e),  Y(cy, -20, e), R(5, e), TFT_WHITE);
}

void ojosCerrados(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.drawLine(X(cx, -36, e), Y(cy, -16, e), X(cx, -16, e), Y(cy, -16, e), TFT_BLACK);
  t.drawLine(X(cx, 16, e),  Y(cy, -16, e), X(cx, 36, e),  Y(cy, -16, e), TFT_BLACK);
}

void ojosFelices(TFT_eSPI& t, int16_t cx, int16_t cy, float e, uint16_t bg) {
  // arcos hacia arriba (^ ^)
  t.drawArc(X(cx, -26, e), Y(cy, -16, e), R(13, e), R(9, e), 90, 270, TFT_BLACK, bg, true);
  t.drawArc(X(cx, 26, e),  Y(cy, -16, e), R(13, e), R(9, e), 90, 270, TFT_BLACK, bg, true);
}

void ojosGuino(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(X(cx, -26, e), Y(cy, -20, e), R(9, e), TFT_BLACK);
  t.drawLine(X(cx, 16, e), Y(cy, -16, e), X(cx, 36, e), Y(cy, -16, e), TFT_BLACK);
}

void cejasEnojadas(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.drawLine(X(cx, -38, e), Y(cy, -42, e), X(cx, -14, e), Y(cy, -30, e), TFT_BLACK);
  t.drawLine(X(cx, 14, e),  Y(cy, -30, e), X(cx, 38, e),  Y(cy, -42, e), TFT_BLACK);
}

// ---------- bocas ----------
void bocaSonrisa(TFT_eSPI& t, int16_t cx, int16_t cy, float e, uint16_t bg) {
  t.drawArc(cx, Y(cy, 24, e), R(28, e), R(20, e), 270, 90, TFT_BLACK, bg, true);
}

void bocaTriste(TFT_eSPI& t, int16_t cx, int16_t cy, float e, uint16_t bg) {
  t.drawArc(cx, Y(cy, 42, e), R(28, e), R(20, e), 90, 270, TFT_BLACK, bg, true);
}

void bocaPlana(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillRect(X(cx, -20, e), Y(cy, 26, e), R(40, e), R(6, e), TFT_BLACK);
}

void bocaAbierta(TFT_eSPI& t, int16_t cx, int16_t cy, float e, int16_t ancho = 22, int16_t alto = 28) {
  t.fillEllipse(cx, Y(cy, 34, e), R(ancho, e), R(alto, e), TFT_BLACK);
  t.fillEllipse(cx, Y(cy, 22, e), R(12, e), R(10, e), TFT_RED);
}

void bocaRisa(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillEllipse(cx, Y(cy, 32, e), R(30, e), R(24, e), TFT_BLACK);
  t.fillEllipse(cx, Y(cy, 20, e), R(14, e), R(8, e), TFT_RED);
}

void bocaOndulada(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  // boca confundida (zig-zag suave)
  t.drawLine(X(cx, -16, e), Y(cy, 32, e), X(cx, -5, e), Y(cy, 26, e), TFT_BLACK);
  t.drawLine(X(cx, -5, e),  Y(cy, 26, e), X(cx, 5, e),  Y(cy, 34, e), TFT_BLACK);
  t.drawLine(X(cx, 5, e),   Y(cy, 34, e), X(cx, 16, e), Y(cy, 28, e), TFT_BLACK);
}

void gotaSudor(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  // gota de sudor (lágrima) azul junto a la cabeza
  t.fillCircle(X(cx, 62, e), Y(cy, -40, e), R(9, e), TFT_BLUE);
  t.fillTriangle(X(cx, 54, e), Y(cy, -40, e), X(cx, 70, e), Y(cy, -40, e), X(cx, 62, e), Y(cy, -62, e), TFT_BLUE);
}

void lagrima(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(X(cx, -56, e), Y(cy, 0, e), R(8, e), TFT_BLUE);
  t.fillTriangle(X(cx, -62, e), Y(cy, 0, e), X(cx, -50, e), Y(cy, 0, e), X(cx, -56, e), Y(cy, -22, e), TFT_BLUE);
}

// ---------- emojis especiales ----------
void dibujarCalabaza(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillEllipse(cx, cy, R(60, e), R(50, e), TFT_ORANGE);
  uint16_t surco = t.color565(200, 100, 0);
  t.drawLine(X(cx, -35, e), Y(cy, -40, e), X(cx, -35, e), Y(cy, 40, e), surco);
  t.drawLine(cx, Y(cy, -45, e), cx, Y(cy, 45, e), surco);
  t.drawLine(X(cx, 35, e), Y(cy, -40, e), X(cx, 35, e), Y(cy, 40, e), surco);
  t.fillRect(X(cx, -8, e), Y(cy, -64, e), R(16, e), R(20, e), TFT_GREEN);
  t.fillTriangle(X(cx, -28, e), Y(cy, -12, e), X(cx, -10, e), Y(cy, -12, e), X(cx, -19, e), Y(cy, 2, e), TFT_BLACK);
  t.fillTriangle(X(cx, 10, e),  Y(cy, -12, e), X(cx, 28, e),  Y(cy, -12, e), X(cx, 19, e),  Y(cy, 2, e), TFT_BLACK);
  t.fillTriangle(X(cx, -25, e), Y(cy, 16, e), X(cx, 25, e), Y(cy, 16, e), cx, Y(cy, 40, e), TFT_BLACK);
}

void dibujarCalavera(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(cx, Y(cy, -10, e), R(55, e), TFT_WHITE);
  t.fillRect(X(cx, -40, e), Y(cy, -5, e), R(80, e), R(42, e), TFT_WHITE);
  t.fillCircle(X(cx, -22, e), Y(cy, -20, e), R(14, e), TFT_BLACK);
  t.fillCircle(X(cx, 22, e),  Y(cy, -20, e), R(14, e), TFT_BLACK);
  t.fillTriangle(X(cx, -7, e), Y(cy, -4, e), X(cx, 7, e), Y(cy, -4, e), cx, Y(cy, 10, e), TFT_BLACK);
  t.drawLine(X(cx, -20, e), Y(cy, 22, e), X(cx, 20, e), Y(cy, 22, e), TFT_BLACK);
  t.drawLine(X(cx, -10, e), Y(cy, 18, e), X(cx, -10, e), Y(cy, 26, e), TFT_BLACK);
  t.drawLine(cx, Y(cy, 18, e), cx, Y(cy, 26, e), TFT_BLACK);
  t.drawLine(X(cx, 10, e), Y(cy, 18, e), X(cx, 10, e), Y(cy, 26, e), TFT_BLACK);
}

void dibujarRobot(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillRoundRect(X(cx, -50, e), Y(cy, -60, e), R(100, e), R(100, e), R(20, e), TFT_DARKGREY);
  t.drawLine(cx, Y(cy, -60, e), cx, Y(cy, -90, e), TFT_DARKGREY);
  t.fillCircle(cx, Y(cy, -92, e), R(6, e), TFT_RED);
  t.fillCircle(X(cx, -20, e), Y(cy, -20, e), R(10, e), TFT_BLUE);
  t.fillCircle(X(cx, 20, e),  Y(cy, -20, e), R(10, e), TFT_BLUE);
  t.fillRect(X(cx, -16, e), Y(cy, 16, e), R(32, e), R(8, e), TFT_BLACK);
}

void dibujarAlien(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(cx, cy, R(75, e), C_ALIEN);
  t.fillEllipse(X(cx, -24, e), Y(cy, -10, e), R(14, e), R(24, e), TFT_BLACK);
  t.fillEllipse(X(cx, 24, e),  Y(cy, -10, e), R(14, e), R(24, e), TFT_BLACK);
  t.fillEllipse(cx, Y(cy, 30, e), R(14, e), R(8, e), TFT_BLACK);
}

void dibujarFantasma(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(cx, Y(cy, -20, e), R(50, e), TFT_WHITE);
  t.fillRect(X(cx, -50, e), Y(cy, -20, e), R(100, e), R(60, e), TFT_WHITE);
  t.fillCircle(X(cx, -32, e), Y(cy, 40, e), R(18, e), TFT_WHITE);
  t.fillCircle(cx, Y(cy, 40, e), R(18, e), TFT_WHITE);
  t.fillCircle(X(cx, 32, e), Y(cy, 40, e), R(18, e), TFT_WHITE);
  t.fillCircle(X(cx, -20, e), Y(cy, -20, e), R(10, e), TFT_BLACK);
  t.fillCircle(X(cx, 20, e),  Y(cy, -20, e), R(10, e), TFT_BLACK);
  t.fillEllipse(cx, Y(cy, 8, e), R(12, e), R(16, e), TFT_BLACK);
}

void dibujarCorazon(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  int16_t r = R(34, e);
  t.fillCircle(X(cx, -20, e), Y(cy, -20, e), r, TFT_RED);
  t.fillCircle(X(cx, 20, e),  Y(cy, -20, e), r, TFT_RED);
  t.fillTriangle(X(cx, -52, e), Y(cy, -8, e), X(cx, 52, e), Y(cy, -8, e), X(cx, 0, e), Y(cy, 72, e), TFT_RED);
}

void dibujarFuego(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  t.fillCircle(cx, Y(cy, 22, e), R(40, e), TFT_ORANGE);
  t.fillTriangle(X(cx, -32, e), Y(cy, 12, e), X(cx, 32, e), Y(cy, 12, e), cx, Y(cy, -72, e), TFT_ORANGE);
  t.fillCircle(cx, Y(cy, 22, e), R(26, e), TFT_YELLOW);
  t.fillTriangle(X(cx, -18, e), Y(cy, 16, e), X(cx, 18, e), Y(cy, 16, e), cx, Y(cy, -44, e), TFT_YELLOW);
}

void dibujarArcoiris(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  const int16_t radios[] = {78, 66, 54, 42, 30, 18};
  const uint16_t colores[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_PURPLE};
  for (int i = 0; i < 6; i++) {
    t.drawArc(cx, Y(cy, 40, e), R(radios[i], e), R(radios[i] - 9, e), 90, 270, colores[i], TFT_BLACK, true);
  }
}

void dibujarDinero(TFT_eSPI& t, int16_t cx, int16_t cy, float e) {
  caraBase(t, cx, cy, e, C_VERDE);
  t.setTextColor(TFT_BLACK, C_VERDE);
  t.drawChar('$', X(cx, -36, e), Y(cy, -26, e), 2);
  t.drawChar('$', X(cx, 24, e),  Y(cy, -26, e), 2);
  bocaSonrisa(t, cx, cy, e, C_VERDE);
}

} // namespace

// ============================================================
//  Despachador principal
// ============================================================
void dibujarEmoji(TFT_eSPI& tft, const char* nombre, int16_t cx, int16_t cy, float escala) {
  float e = escala;

  // ----- caras amarillas estándar -----
  if (strcmp(nombre, "feliz") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosFelices(tft, cx, cy, e, C_AMARILLO);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "enojado") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    cejasEnojadas(tft, cx, cy, e);
    ojosPuntos(tft, cx, cy, e);
    bocaTriste(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "triste") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosPuntos(tft, cx, cy, e);
    bocaTriste(tft, cx, cy, e, C_AMARILLO);
    lagrima(tft, cx, cy, e);
  } else if (strcmp(nombre, "nerd") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    // gafas
    tft.drawRoundRect(X(cx, -38, e), Y(cy, -30, e), R(30, e), R(26, e), R(6, e), TFT_BLACK);
    tft.drawRoundRect(X(cx, 8, e),   Y(cy, -30, e), R(30, e), R(26, e), R(6, e), TFT_BLACK);
    tft.drawLine(X(cx, -8, e), Y(cy, -17, e), X(cx, 8, e), Y(cy, -17, e), TFT_BLACK);
    ojosPuntos(tft, cx, cy, e);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
    tft.fillRect(X(cx, -10, e), Y(cy, 26, e), R(20, e), R(8, e), TFT_WHITE);
  } else if (strcmp(nombre, "guiño") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosGuino(tft, cx, cy, e);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "meh") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosPuntos(tft, cx, cy, e);
    bocaPlana(tft, cx, cy, e);
  } else if (strcmp(nombre, "enamorado") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    corazonChico(tft, X(cx, -26, e), Y(cy, -20, e), e, TFT_RED);
    corazonChico(tft, X(cx, 26, e),  Y(cy, -20, e), e, TFT_RED);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "risa") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosFelices(tft, cx, cy, e, C_AMARILLO);
    bocaRisa(tft, cx, cy, e);
  } else if (strcmp(nombre, "sorpresa") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosGrandes(tft, cx, cy, e);
    tft.fillEllipse(cx, Y(cy, 34, e), R(18, e), R(22, e), TFT_BLACK);
  } else if (strcmp(nombre, "miedo") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosGrandes(tft, cx, cy, e);
    tft.fillEllipse(cx, Y(cy, 32, e), R(16, e), R(12, e), TFT_BLACK);
    gotaSudor(tft, cx, cy, e);
  } else if (strcmp(nombre, "dormido") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosCerrados(tft, cx, cy, e);
    tft.fillEllipse(cx, Y(cy, 32, e), R(10, e), R(8, e), TFT_BLACK);
    tft.setTextColor(TFT_BLUE, C_AMARILLO);
    tft.drawChar('Z', X(cx, 46, e), Y(cy, -60, e), 2);
    tft.drawChar('Z', X(cx, 58, e), Y(cy, -78, e), 4);
  } else if (strcmp(nombre, "pensativo") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosPuntos(tft, cx, cy, e);
    // ceja levantada
    tft.drawLine(X(cx, -38, e), Y(cy, -34, e), X(cx, -14, e), Y(cy, -26, e), TFT_BLACK);
    bocaPlana(tft, cx, cy, e);
  } else if (strcmp(nombre, "cool") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    // gafas de sol
    tft.fillRoundRect(X(cx, -46, e), Y(cy, -28, e), R(38, e), R(22, e), R(6, e), TFT_BLACK);
    tft.fillRoundRect(X(cx, 8, e),   Y(cy, -28, e), R(38, e), R(22, e), R(6, e), TFT_BLACK);
    tft.fillRect(X(cx, -8, e), Y(cy, -22, e), R(16, e), R(4, e), TFT_BLACK);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "halo") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    tft.drawEllipse(cx, Y(cy, -92, e), R(40, e), R(14, e), TFT_YELLOW);
    ojosFelices(tft, cx, cy, e, C_AMARILLO);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "beso") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosCerrados(tft, cx, cy, e);
    // boquita de beso (corazón)
    corazonChico(tft, cx, Y(cy, 28, e), e, TFT_RED);
  } else if (strcmp(nombre, "estrellas") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    estrella(tft, X(cx, -26, e), Y(cy, -20, e), e, 13, TFT_BLACK);
    estrella(tft, X(cx, 26, e),  Y(cy, -20, e), e, 13, TFT_BLACK);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
  } else if (strcmp(nombre, "grito") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosGrandes(tft, cx, cy, e);
    tft.fillEllipse(cx, Y(cy, 34, e), R(28, e), R(34, e), TFT_BLACK);
    tft.fillEllipse(cx, Y(cy, 22, e), R(14, e), R(10, e), TFT_RED);
  } else if (strcmp(nombre, "confundido") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosPuntos(tft, cx, cy, e);
    bocaOndulada(tft, cx, cy, e);
  } else if (strcmp(nombre, "aliviado") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosCerrados(tft, cx, cy, e);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
    gotaSudor(tft, cx, cy, e);
  } else if (strcmp(nombre, "abrazo") == 0) {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosFelices(tft, cx, cy, e, C_AMARILLO);
    bocaSonrisa(tft, cx, cy, e, C_AMARILLO);
    // brazos abiertos
    tft.drawArc(cx, cy, R(95, e), R(85, e), 200, 340, TFT_ORANGE, TFT_BLACK, true);
  }

  // ----- emojis especiales -----
  else if (strcmp(nombre, "calabaza") == 0) {
    dibujarCalabaza(tft, cx, cy, e);
  } else if (strcmp(nombre, "calavera") == 0) {
    dibujarCalavera(tft, cx, cy, e);
  } else if (strcmp(nombre, "robot") == 0) {
    dibujarRobot(tft, cx, cy, e);
  } else if (strcmp(nombre, "alien") == 0) {
    dibujarAlien(tft, cx, cy, e);
  } else if (strcmp(nombre, "fantasma") == 0) {
    dibujarFantasma(tft, cx, cy, e);
  } else if (strcmp(nombre, "corazon") == 0) {
    dibujarCorazon(tft, cx, cy, e);
  } else if (strcmp(nombre, "fuego") == 0) {
    dibujarFuego(tft, cx, cy, e);
  } else if (strcmp(nombre, "estrella") == 0) {
    estrella(tft, cx, cy, e, 75, TFT_YELLOW);
  } else if (strcmp(nombre, "arcoiris") == 0) {
    dibujarArcoiris(tft, cx, cy, e);
  } else if (strcmp(nombre, "dinero") == 0) {
    dibujarDinero(tft, cx, cy, e);
  }

  // ----- emoji desconocido: cara genérica -----
  else {
    caraBase(tft, cx, cy, e, C_AMARILLO);
    ojosPuntos(tft, cx, cy, e);
    bocaPlana(tft, cx, cy, e);
  }
}
