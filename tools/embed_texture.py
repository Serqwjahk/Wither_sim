#!/usr/bin/env python3
"""Convierte assets/wither.png en src/render/wither_texture.h (RGBA8 incrustado).

Uso:  python3 tools/embed_texture.py
Requiere Pillow (pip install pillow). Solo hace falta volver a correrlo si
cambias la textura; el header generado ya viene en el repo.

Por que incrustarla: asi no dependemos de stb_image, de rutas de archivos ni de
romfs para los hitos 1-2 (funciona igual en PC y en Switch).
"""
import os
import sys
from PIL import Image

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = os.path.join(root, "assets", "wither.png")
dst = os.path.join(root, "src", "render", "wither_texture.h")

im = Image.open(src).convert("RGBA")
w, h = im.size
data = im.tobytes()

lines = []
for i in range(0, len(data), 16):
    lines.append("    " + ", ".join("0x%02x" % b for b in data[i:i + 16]) + ",")

with open(dst, "w") as f:
    f.write("// GENERADO por tools/embed_texture.py a partir de assets/wither.png. No editar.\n")
    f.write("#pragma once\n#include <cstdint>\n\n")
    f.write("namespace embedded {\n")
    f.write("constexpr int kWitherTexW = %d;\nconstexpr int kWitherTexH = %d;\n" % (w, h))
    f.write("// RGBA8, fila 0 = arriba (igual que el PNG)\n")
    f.write("constexpr uint8_t kWitherTexRGBA[%d] = {\n" % len(data))
    f.write("\n".join(lines))
    f.write("\n};\n}  // namespace embedded\n")

print("OK:", dst, "(%dx%d)" % (w, h))
sys.exit(0)
