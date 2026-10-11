#include "Game.h"

#include <cmath>

namespace wm {

namespace {
constexpr float kHoverHeight = 1.6f;      // altura base sobre el suelo, en bloques
constexpr float kTargetSpeed = 6.0f;      // bloques/s
constexpr float kTargetRange = 22.0f;     // radio maximo del objetivo
constexpr float kBodyTurnSpeed = 90.0f;   // grados/s (el cuerpo gira lento)
constexpr float kHeadTurnSpeed[3] = {420.0f, 300.0f, 300.0f};  // grados/s
constexpr float kMaxHeadYaw = 100.0f;     // grados respecto al cuerpo
constexpr float kMaxHeadPitch = 60.0f;
}  // namespace

Game::Game() { reset(); }

void Game::reset() {
    time_ = 0.0f;
    bodyYaw_ = 0.0f;
    witherPos_ = Vec3(0, kHoverHeight, 0);
    target_ = Vec3(0, 3.0f, -8.0f);   // delante del wither (mira hacia -Z)
    autoTargets_ = false;
    camera_ = Camera();
    markers_[0].color = Vec3(1.0f, 0.9f, 0.2f);
    markers_[1].color = Vec3(0.2f, 0.9f, 1.0f);
    markers_[2].color = Vec3(1.0f, 0.5f, 0.1f);
    updateTargets(0.0f, InputState());
    updateWither(0.0f);
}

void Game::updateTargets(float dt, const InputState& in) {
    if (autoTargets_) {
        // El objetivo central pasea; asi se ve el comportamiento sin tocar nada.
        target_ = Vec3(std::sin(time_ * 0.55f) * 9.0f,
                       3.5f + std::sin(time_ * 0.9f) * 2.5f,
                       -8.0f + std::cos(time_ * 0.35f) * 5.0f);
    } else {
        Vec3 f = camera_.forwardXZ();
        Vec3 r = camera_.rightXZ();
        Vec3 d = r * in.move.x + f * in.move.y;
        target_ += d * (kTargetSpeed * dt);
        target_.y += in.rise * kTargetSpeed * dt;
        target_.y = clampf(target_.y, 0.3f, 14.0f);
        // Limitar al circulo de juego (en el plano XZ) alrededor del wither.
        float dx = target_.x - witherPos_.x, dz = target_.z - witherPos_.z;
        float len = std::sqrt(dx * dx + dz * dz);
        if (len > kTargetRange) {
            target_.x = witherPos_.x + dx / len * kTargetRange;
            target_.z = witherPos_.z + dz / len * kTargetRange;
        }
    }

    // Las cabezas laterales miran a dos puntos que orbitan alrededor del central:
    // "cada cabeza apunta a algo distinto".
    markers_[0].position = target_;
    markers_[1].position = target_ + Vec3(std::cos(time_ * 1.3f) * 4.5f, std::sin(time_ * 0.9f) * 2.0f,
                                          std::sin(time_ * 1.3f) * 4.5f);
    markers_[2].position = target_ + Vec3(std::cos(time_ * 1.1f + kPi) * 4.5f, std::sin(time_ * 1.4f) * 2.0f,
                                          std::sin(time_ * 1.1f + kPi) * 4.5f);
}

void Game::updateWither(float dt) {
    witherPos_.y = kHoverHeight + std::sin(time_ * 1.3f) * 0.12f;

    // El cuerpo gira (despacio) hacia el objetivo central, solo en horizontal.
    Vec3 toTarget = target_ - witherPos_;
    if (toTarget.x * toTarget.x + toTarget.z * toTarget.z > 0.01f) {
        // Frente del modelo = (-sin yaw, 0, -cos yaw)  =>  yaw = atan2(-dx, -dz)
        float wantYaw = std::atan2(-toTarget.x, -toTarget.z);
        bodyYaw_ = approachAngle(bodyYaw_, wantYaw, radians(kBodyTurnSpeed) * dt);
    }

    wither_.animateIdle(time_);

    // Cada cabeza apunta a su objetivo (calculado en el espacio del modelo).
    for (int h = 0; h < 3; ++h) {
        Vec3 pivot = wither_.headPivotJava(h);
        Vec3 targetJ = WitherModel::worldToJava(markers_[h].position, witherPos_, bodyYaw_);
        Vec3 d = normalize(targetJ - pivot);
        // Forward local = -Z. Con R = Ry(b)*Rx(a):  d = (-sin b cos a, sin a, -cos b cos a)
        float wantPitch = std::asin(clampf(d.y, -1.0f, 1.0f));
        float wantYaw = std::atan2(-d.x, -d.z);
        wantPitch = clampf(wantPitch, -radians(kMaxHeadPitch), radians(kMaxHeadPitch));
        wantYaw = clampf(wantYaw, -radians(kMaxHeadYaw), radians(kMaxHeadYaw));

        float step = radians(kHeadTurnSpeed[h]) * dt;
        float yaw = approachAngle(wither_.headYaw(h), wantYaw, step);
        float pitch = approachAngle(wither_.headPitch(h), wantPitch, step);
        wither_.setHeadRotation(h, yaw, pitch);
    }

    Mat4 root = translate(witherPos_) * rotateY(bodyYaw_) *
                scale(Vec3(WitherModel::kScale, WitherModel::kScale, WitherModel::kScale));
    wither_.updateTransforms(root);
}

void Game::update(float dt, const InputState& in) {
    if (dt > 0.1f) dt = 0.1f;  // evita saltos si la ventana se congela
    if (in.resetPressed) {
        reset();
        return;
    }
    if (in.toggleAuto) autoTargets_ = !autoTargets_;
    time_ += dt;

    camera_.orbit(in.orbit.x * 2.0f * dt + in.orbitDelta.x, in.orbit.y * 1.5f * dt + in.orbitDelta.y);
    camera_.zoom(in.zoom * 1.2f * dt + in.zoomDelta * 0.1f);

    updateTargets(dt, in);
    updateWither(dt);
    camera_.center = witherPos_ + Vec3(0, 1.8f, 0);
}

}  // namespace wm
