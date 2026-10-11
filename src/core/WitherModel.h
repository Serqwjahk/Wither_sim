// Modelo del Wither: 6 partes (9 cubos) con jerarquia, igual que el wither.java
// exportado de Blockbench (formato Mojang mappings). Esta clase NO sabe nada de
// OpenGL ni de la plataforma: solo produce malla (CPU) y matrices.
#pragma once
#include <cstdint>
#include <vector>

#include "Math.h"

namespace wm {

struct Vertex {
    Vec3 pos;     // en espacio local de la parte (relativo a su pivote)
    Vec3 normal;
    Vec2 uv;      // 0..1 sobre la textura 64x64
};

struct Part {
    const char* name = "";
    int parent = -1;
    Vec3 offset;               // PartPose.offset (espacio "java": y hacia abajo)
    Vec3 rot;                  // rotacion actual en radianes (x, y, z)
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    Mat4 world = Mat4::identity();  // calculada en updateTransforms()
};

class WitherModel {
public:
    enum PartId : int { Shoulders, Ribcage, Tail, HeadCenter, HeadLeft, HeadRight, PartCount };

    // Los datos estan en "pixeles de Minecraft". 1 bloque = 16 px y el wither se
    // dibuja al doble de tamano en el juego original, de ahi 2/16.
    static constexpr float kScale = 2.0f / 16.0f;

    WitherModel();

    // Animacion de reposo: balanceo de costillas y cola (como WitherBossModel).
    void animateIdle(float timeSeconds);

    // head: 0 = central, 1 = izquierda, 2 = derecha. Angulos en radianes, en el
    // espacio del modelo (yaw alrededor de Y, pitch alrededor de X).
    void setHeadRotation(int head, float yaw, float pitch);
    float headYaw(int head) const { return parts_[kHeadParts[head]].rot.y; }
    float headPitch(int head) const { return parts_[kHeadParts[head]].rot.x; }

    // Pivote de la cabeza en espacio "java" (relativo al hombro/raiz).
    Vec3 headPivotJava(int head) const { return parts_[kHeadParts[head]].offset; }

    // Pivote de la cabeza en el mundo (valido tras updateTransforms).
    Vec3 headPivotWorld(int head) const;

    // rootWorld = traslacion * rotacionY * escala uniforme kScale. Aqui se
    // aplica ademas el cambio de ejes java -> mundo, scale(-1,-1,1).
    void updateTransforms(const Mat4& rootWorld);

    // Convierte un punto del mundo al espacio "java" del modelo (inverso de lo
    // anterior). Sirve para calcular hacia donde mirar.
    static Vec3 worldToJava(const Vec3& world, const Vec3& position, float yaw);

    const std::vector<Part>& parts() const { return parts_; }

private:
    static constexpr int kHeadParts[3] = {HeadCenter, HeadLeft, HeadRight};

    int addPart(const char* name, int parent, Vec3 offset, Vec3 rot);
    // Equivale a CubeListBuilder.texOffs(u,v).addBox(x,y,z,w,h,d)
    void addBox(int part, float x, float y, float z, float w, float h, float d, float texU, float texV);

    std::vector<Part> parts_;
};

}  // namespace wm
