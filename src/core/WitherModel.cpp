#include "WitherModel.h"

#include <cmath>

namespace wm {

namespace {
constexpr float kTexW = 64.0f;
constexpr float kTexH = 64.0f;

// Una cara de un cubo: 4 vertices en orden de ModelBox de Minecraft y el
// rectangulo de textura (u1,v1)-(u2,v2). Ver addBox() para la tabla.
struct Quad {
    Vec3 p[4];
    Vec3 normal;
    float u1, v1, u2, v2;
};
}  // namespace

int WitherModel::addPart(const char* name, int parent, Vec3 offset, Vec3 rot) {
    Part p;
    p.name = name;
    p.parent = parent;
    p.offset = offset;
    p.rot = rot;
    parts_.push_back(p);
    return static_cast<int>(parts_.size()) - 1;
}

void WitherModel::addBox(int part, float x, float y, float z, float w, float h, float d,
                         float U, float V) {
    const float x0 = x, x1 = x + w;
    const float y0 = y, y1 = y + h;
    const float z0 = z, z1 = z + d;

    // Mismos vertices que ModelBox: v0 = (min,min,min) ... v7 = (min,max,max).
    // Ojo: en espacio java "y minima" es ARRIBA (la y crece hacia abajo).
    const Vec3 v0(x0, y0, z0), v1(x1, y0, z0), v2(x1, y1, z0), v3(x0, y1, z0);
    const Vec3 v4(x0, y0, z1), v5(x1, y0, z1), v6(x1, y1, z1), v7(x0, y1, z1);

    // Distribucion UV estandar de Minecraft ("box UV"):
    //            [ arriba ][ abajo  ]
    //   [ -X ][ frente(-Z) ][ +X ][ detras(+Z) ]
    const Quad quads[6] = {
        {{v5, v1, v2, v6}, Vec3(1, 0, 0),  U + d + w,         V + d, U + d + w + d,     V + d + h},  // +X
        {{v0, v4, v7, v3}, Vec3(-1, 0, 0), U,                 V + d, U + d,             V + d + h},  // -X
        {{v5, v4, v0, v1}, Vec3(0, -1, 0), U + d,             V,     U + d + w,         V + d},      // arriba
        {{v2, v3, v7, v6}, Vec3(0, 1, 0),  U + d + w,         V,     U + d + w + w,     V + d},      // abajo
        {{v1, v0, v3, v2}, Vec3(0, 0, -1), U + d,             V + d, U + d + w,         V + d + h},  // frente
        {{v4, v5, v6, v7}, Vec3(0, 0, 1),  U + d + w + d,     V + d, U + d + w + d + w, V + d + h},  // detras
    };

    Part& pt = parts_[part];
    for (const Quad& q : quads) {
        // Cada vertice recibe su esquina de la textura (igual que TexturedQuad).
        const float us[4] = {q.u2, q.u1, q.u1, q.u2};
        const float vs[4] = {q.v1, q.v1, q.v2, q.v2};

        // Orden de los vertices: lo forzamos a antihorario visto desde fuera
        // comparando con la normal, para no depender de recordar el orden exacto.
        Vec3 n = cross(q.p[1] - q.p[0], q.p[2] - q.p[0]);
        const bool flip = dot(n, q.normal) < 0.0f;

        const uint16_t base = static_cast<uint16_t>(pt.vertices.size());
        for (int i = 0; i < 4; ++i) {
            int k = flip ? (3 - i) : i;
            Vertex vtx;
            vtx.pos = q.p[k];
            vtx.normal = q.normal;
            vtx.uv = {us[k] / kTexW, vs[k] / kTexH};
            pt.vertices.push_back(vtx);
        }
        const uint16_t idx[6] = {0, 1, 2, 0, 2, 3};
        for (uint16_t i : idx) pt.indices.push_back(static_cast<uint16_t>(base + i));
    }
}

WitherModel::WitherModel() {
    // Datos tomados tal cual de wither.java (createBodyLayer). La unica
    // diferencia: el hombro usa offset (0,0,0) en vez de (0,24,0); ese 24 es la
    // altura base de Minecraft y aqui lo ponemos nosotros en Game.
    const int shoulders = addPart("upperBodyPart1", -1, Vec3(0, 0, 0), Vec3(0, 0, 0));
    addBox(shoulders, -10.0f, -20.1f, -0.5f, 20, 3, 3, 0, 16);

    const int ribcage = addPart("upperBodyPart2", shoulders, Vec3(-2.0f, -17.1f, -0.5f), Vec3(0, 0, 0));
    addBox(ribcage, 0.0f, 0.0f, 0.0f, 3, 10, 3, 0, 22);
    addBox(ribcage, -4.0f, 1.5f, 0.5f, 11, 2, 2, 24, 22);
    addBox(ribcage, -4.0f, 4.0f, 0.5f, 11, 2, 2, 24, 22);
    addBox(ribcage, -4.0f, 6.5f, 0.5f, 11, 2, 2, 24, 22);

    const int tail = addPart("upperBodyPart3", ribcage, Vec3(2.0f, 10.1f, 0.5f), Vec3(0.7854f, 0, 0));
    addBox(tail, -2.0f, 0.0f, 0.0f, 3, 6, 3, 12, 22);

    const int head1 = addPart("head1", shoulders, Vec3(0.0f, -20.0f, 0.0f), Vec3(0, 0, 0));
    addBox(head1, -4.0f, -8.0f, -4.0f, 8, 8, 8, 0, 0);

    const int head2 = addPart("head2", shoulders, Vec3(-9.0f, -18.0f, -1.0f), Vec3(0, 0, 0));
    addBox(head2, -3.0f, -6.0f, -3.0f, 6, 6, 6, 32, 0);

    const int head3 = addPart("head3", shoulders, Vec3(9.0f, -18.0f, -1.0f), Vec3(0, 0, 0));
    addBox(head3, -3.0f, -6.0f, -3.0f, 6, 6, 6, 32, 0);

    // El orden de insercion debe coincidir con el enum PartId.
    (void)tail; (void)head1; (void)head2; (void)head3;
}

void WitherModel::animateIdle(float t) {
    // WitherBossModel usa cos(ageInTicks * 0.1) con 20 ticks/s => 2 rad/s.
    const float f = std::cos(t * 2.0f);
    parts_[Ribcage].rot.x = (0.065f + 0.05f * f) * kPi;
    // La cola ya cuelga de las costillas, asi que su rotacion es relativa.
    parts_[Tail].rot.x = (0.25f + 0.05f * f) * kPi;
}

void WitherModel::setHeadRotation(int head, float yaw, float pitch) {
    Part& p = parts_[kHeadParts[head]];
    p.rot.y = yaw;
    p.rot.x = pitch;
}

void WitherModel::updateTransforms(const Mat4& rootWorld) {
    // java -> mundo: Blockbench/Minecraft usan y hacia abajo y x invertida.
    const Mat4 javaToWorld = rootWorld * scale(Vec3(-1, -1, 1));
    for (Part& p : parts_) {
        // Igual que ModelPart.translateAndRotate: traslacion y luego Z, Y, X.
        Mat4 local = translate(p.offset) * rotateZ(p.rot.z) * rotateY(p.rot.y) * rotateX(p.rot.x);
        p.world = (p.parent < 0 ? javaToWorld : parts_[p.parent].world) * local;
    }
}

Vec3 WitherModel::headPivotWorld(int head) const {
    return transformPoint(parts_[kHeadParts[head]].world, Vec3(0, 0, 0));
}

Vec3 WitherModel::worldToJava(const Vec3& world, const Vec3& position, float yaw) {
    Vec3 v = world - position;
    v = transformPoint(rotateY(-yaw), v);  // deshace la guinada del cuerpo
    v = v * (1.0f / kScale);               // deshace la escala
    return Vec3(-v.x, -v.y, v.z);          // mundo -> java
}

}  // namespace wm
