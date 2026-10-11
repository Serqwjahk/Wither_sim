// Prueba sin ventana: renderiza el juego con EGL + pbuffer (Mesa software) y
// guarda un PPM. EGL se declara a mano para no necesitar sus headers.
//   offscreen_test salida.ppm [segundos=0] [auto=0] [yawCamGrados=0] [pitchCamGrados=0]
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "../src/core/Game.h"
#include "../src/render/Renderer.h"

extern "C" {
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLSurface;
typedef int EGLint;
typedef unsigned int EGLenum;
typedef unsigned int EGLBoolean;
EGLDisplay eglGetDisplay(void*);
EGLBoolean eglInitialize(EGLDisplay, EGLint*, EGLint*);
EGLBoolean eglBindAPI(EGLenum);
EGLBoolean eglChooseConfig(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
EGLSurface eglCreatePbufferSurface(EGLDisplay, EGLConfig, const EGLint*);
EGLContext eglCreateContext(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
EGLBoolean eglMakeCurrent(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
void* eglGetProcAddress(const char*);
EGLint eglGetError();
}

static void* getProc(const char* n) { return eglGetProcAddress(n); }

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "uso: %s out.ppm [seg] [auto] [yaw] [pitch]\n", argv[0]); return 2; }
    const float seconds = argc > 2 ? std::atof(argv[2]) : 0.0f;
    const bool autoMode = argc > 3 && std::atoi(argv[3]) != 0;
    const float camYaw = argc > 4 ? std::atof(argv[4]) : 0.0f;
    const float camPitch = argc > 5 ? std::atof(argv[5]) : 0.0f;
    const int W = 960, H = 540;

    EGLDisplay dpy = eglGetDisplay(nullptr);
    EGLint maj, min;
    if (!eglInitialize(dpy, &maj, &min)) { std::fprintf(stderr, "eglInitialize fallo 0x%x\n", eglGetError()); return 1; }
    eglBindAPI(0x30A2);  // EGL_OPENGL_API
    const EGLint cfgAttr[] = {0x3033, 0x0001, 0x3040, 0x0008, 0x3024, 8, 0x3023, 8, 0x3022, 8, 0x3021, 8, 0x3025, 24, 0x3038};
    EGLConfig cfg; EGLint n = 0;
    if (!eglChooseConfig(dpy, cfgAttr, &cfg, 1, &n) || n < 1) { std::fprintf(stderr, "sin config\n"); return 1; }
    const EGLint pbAttr[] = {0x3057, W, 0x3056, H, 0x3038};
    EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbAttr);
    const EGLint ctxAttr[] = {0x3098, 3, 0x30FB, 3, 0x30FD, 1, 0x3038};
    EGLContext ctx = eglCreateContext(dpy, cfg, nullptr, ctxAttr);
    if (!ctx || !eglMakeCurrent(dpy, surf, surf, ctx)) { std::fprintf(stderr, "sin contexto 3.3 core 0x%x\n", eglGetError()); return 1; }

    wm::Renderer r;
    std::string err;
    if (!r.init(getProc, err)) { std::fprintf(stderr, "init fallo: %s\n", err.c_str()); return 1; }
    std::printf("GL: %s | %s\n", (const char*)gl::GetString(gl::VERSION), (const char*)gl::GetString(gl::RENDERER));

    wm::Game game;
    wm::InputState in;
    in.toggleAuto = autoMode;
    game.update(0.0f, in);
    in.toggleAuto = false;
    // Camara: yaw += camYaw grados.
    in.orbitDelta.x = wm::radians(camYaw);
    in.orbitDelta.y = wm::radians(camPitch);
    game.update(1.0f / 60.0f, in);
    in.orbitDelta = wm::Vec2();
    const int steps = static_cast<int>(seconds * 60.0f);
    for (int i = 0; i < steps; ++i) game.update(1.0f / 60.0f, in);

    // Comprobacion numerica: hacia donde apunta cada cabeza vs hacia su objetivo.
    const char* names[3] = {"central", "izquierda", "derecha"};
    for (int h = 0; h < 3; ++h) {
        const wm::Part& p = game.wither().parts()[wm::WitherModel::HeadCenter + h];
        wm::Vec3 o = wm::transformPoint(p.world, wm::Vec3(0, 0, 0));
        wm::Vec3 f = wm::normalize(wm::transformPoint(p.world, wm::Vec3(0, 0, -1)) - o);
        wm::Vec3 t = wm::normalize(game.markers()[h].position - o);
        float ang = wm::degrees(std::acos(wm::clampf(wm::dot(f, t), -1.0f, 1.0f)));
        std::printf("cabeza %-9s error de punteria: %6.2f grados (yaw=%.1f pitch=%.1f)\n", names[h], ang,
                    wm::degrees(game.wither().headYaw(h)), wm::degrees(game.wither().headPitch(h)));
    }

    r.render(game, W, H);
    std::vector<unsigned char> px(W * H * 4);
    gl::PixelStorei(gl::PACK_ALIGNMENT, 1);
    gl::ReadPixels(0, 0, W, H, gl::RGBA, gl::UNSIGNED_BYTE, px.data());
    if (gl::GetError() != 0) std::fprintf(stderr, "aviso: glGetError != 0\n");

    FILE* f = std::fopen(argv[1], "wb");
    std::fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = H - 1; y >= 0; --y)
        for (int x = 0; x < W; ++x) std::fwrite(&px[(y * W + x) * 4], 1, 3, f);
    std::fclose(f);
    std::printf("escrito %s\n", argv[1]);
    return 0;
}
