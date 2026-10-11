// Logica del simulador (hitos 1-2): un wither flotando cuyas 3 cabezas miran a
// 3 objetivos distintos. No incluye nada de OpenGL ni de SDL.
#pragma once
#include "Camera.h"
#include "Input.h"
#include "Math.h"
#include "WitherModel.h"

namespace wm {

struct Marker {
    Vec3 position;
    Vec3 color;
};

class Game {
public:
    Game();

    void reset();
    void update(float dt, const InputState& in);

    const WitherModel& wither() const { return wither_; }
    const Camera& camera() const { return camera_; }
    const Marker* markers() const { return markers_; }  // 3: central, izquierda, derecha
    bool autoTargets() const { return autoTargets_; }

private:
    void updateTargets(float dt, const InputState& in);
    void updateWither(float dt);

    WitherModel wither_;
    Camera camera_;
    Marker markers_[3];

    Vec3 witherPos_;
    float bodyYaw_ = 0.0f;
    float time_ = 0.0f;
    Vec3 target_;   // objetivo controlado por el jugador (cabeza central)
    bool autoTargets_ = false;
};

}  // namespace wm
