# Wither Sim (hitos 1 y 2)

Un wither flotando en un suelo de damero. Sus 3 cabezas miran cada una a un objetivo
distinto (cubos de colores), el cuerpo gira despacio hacia el central, y las costillas y
la cola se balancean como en el juego original.

C++17 + SDL2 + OpenGL 3.3 core. **Sin GLM, glad ni stb**: las matematicas (`core/Math.h`),
el cargador de OpenGL (`render/gl_loader.*`) y la textura (`render/wither_texture.h`,
generada con `tools/embed_texture.py`) van dentro del proyecto. La unica dependencia es SDL2.

## Estructura

```
src/core/      logica pura, sin SDL ni OpenGL (modelo, animacion, camara, juego)
src/render/    OpenGL 3.3 core (shaders, mallas, textura)
src/platform/  main_sdl.cpp: ventana, contexto, teclado/raton/mando  <- unico archivo con SDL
tests/         offscreen_test.cpp: renderiza a un PPM sin ventana (Linux + Mesa)
tools/         embed_texture.py: assets/wither.png -> header
CMakeLists.txt   Windows / Linux / macOS
Makefile.switch  Nintendo Switch (devkitPro)
```

## Compilar en PC

```
cmake -B build
cmake --build build
./build/wither-sim
```
SDL2: `apt install libsdl2-dev` / `brew install sdl2` / en Windows vcpkg (`vcpkg install sdl2`)
o el paquete devel de libsdl.org con `-DSDL2_DIR=<ruta>/cmake`.

## Compilar para Switch

```
dkp-pacman -S switch-dev switch-sdl2
export DEVKITPRO=/opt/devkitpro
make -f Makefile.switch
```
Copia `wither-sim.nro` a `/switch/wither-sim/` en la SD. `nxlink -s wither-sim.nro` muestra
los mensajes de error por red (quita `-DWM_NXLINK` del Makefile para la version final).

## Controles

| Accion | Teclado / raton | Mando / Switch |
|---|---|---|
| Mover el objetivo | WASD | Stick izquierdo |
| Objetivo arriba / abajo | E / Q | R / L |
| Girar camara | Arrastrar con el raton, o flechas | Stick derecho |
| Zoom | Rueda, o Z / X | ZL / ZR, o cruceta arriba / abajo |
| Objetivos automaticos si/no | Tab | A |
| Reiniciar | R | Y |
| Pantalla completa | F11 | - |
| Salir | Esc | + |

## Como esta hecho el modelo

`core/WitherModel.cpp` repite `createBodyLayer()` de tu `wither.java` (mismos offsets,
cubos y `texOffs`), con la misma jerarquia: hombros -> costillas -> cola, y 3 cabezas
colgando de los hombros. Las caras de cada cubo usan la misma distribucion UV de caja que
Minecraft. Para convertir al mundo se aplica `scale(-1,-1,1)` (Minecraft usa y hacia abajo
y x invertida) y el factor 2/16 (1 bloque = 16 px, el wither se dibuja al doble).

## Que esta probado y que no

- Probado aqui: el nucleo + renderer compilan sin avisos y se renderizan con Mesa (GL 4.5 core);
  texturas y orientacion revisadas a ojo, y la punteria de las cabezas medida (error < 0.1 grados).
- **No probado**: `main_sdl.cpp` (solo revisado contra una cabecera falsa, sin SDL real),
  el build de CMake completo, y todo lo de Switch (`Makefile.switch`, indices de botones de los
  Joy-Con). Es lo primero que hay que correr; si algo falla, seguramente sera ahi.

## Siguiente (hito 3)

Proyectiles (calaveras) desde cada cabeza, vida y fases, HUD (hace falta una fuente: stb_truetype o bitmap).
