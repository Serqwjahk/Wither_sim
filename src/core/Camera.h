// Camara orbital alrededor de un punto.
#pragma once
#include <cmath>

#include "Math.h"

namespace wm {

struct Camera {
    float yaw = kPi;       // pi => la camara empieza delante del wither (-Z)
    float pitch = 0.25f;   // radianes, limitado a +-85 grados
    float distance = 7.5f;
    Vec3 center = Vec3(0, 2, 0);

    void orbit(float dYaw, float dPitch) {
        yaw = wrapPi(yaw + dYaw);
        pitch = clampf(pitch + dPitch, radians(-85.0f), radians(85.0f));
    }
    void zoom(float amount) {  // amount > 0 aleja (cambio multiplicativo)
        distance = clampf(distance * std::exp(amount), 3.0f, 40.0f);
    }
    Vec3 eye() const {
        float cp = std::cos(pitch);
        return center + Vec3(std::sin(yaw) * cp, std::sin(pitch), std::cos(yaw) * cp) * distance;
    }
    Mat4 view() const { return lookAt(eye(), center, Vec3(0, 1, 0)); }

    // Direcciones horizontales relativas a la camara (para mover el objetivo).
    Vec3 forwardXZ() const { return Vec3(-std::sin(yaw), 0, -std::cos(yaw)); }
    Vec3 rightXZ() const { return Vec3(std::cos(yaw), 0, -std::sin(yaw)); }
};

}  // namespace wm
