// Entrada abstracta. La capa de plataforma (SDL) traduce teclado, raton y
// mandos (incluidos los Joy-Con) a esta estructura; el juego no sabe de donde viene.
#pragma once
#include "Math.h"

namespace wm {

struct InputState {
    Vec2 move;           // -1..1: mover el objetivo (x = derecha, y = alejarse de la camara)
    float rise = 0.0f;   // -1..1: subir/bajar el objetivo
    Vec2 orbit;          // -1..1: girar la camara como un stick (x: derecha, y: abajo = sube la camara)
    Vec2 orbitDelta;     // radianes ya calculados este frame (arrastre de raton)
    float zoom = 0.0f;   // -1..1 como stick, + = alejar
    float zoomDelta = 0.0f;  // pasos de rueda del raton, + = alejar

    bool toggleAuto = false;   // pulsado este frame: objetivos automaticos si/no
    bool resetPressed = false; // pulsado este frame: reiniciar la escena
    bool quit = false;
};

}  // namespace wm
