// Capa de plataforma: ventana + contexto OpenGL 3.3 core + entrada con SDL2.
// Es el UNICO archivo que conoce SDL; el mismo codigo sirve para Windows,
// Linux, macOS y Nintendo Switch (devkitPro, portlib switch-sdl2).
#include <SDL.h>

#include <cmath>
#include <cstdio>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "../core/Game.h"
#include "../render/Renderer.h"

namespace {

// ---------------------------------------------------------------------------
// Mando. En PC usamos la API SDL_GameController (mapeo estandar). En Switch
// usamos la API SDL_Joystick con los indices de libnx, que es lo que usan los
// ejemplos oficiales de devkitPro y funciona igual con Joy-Con y Pro Controller.
// ---------------------------------------------------------------------------
struct PadState {
    float lx = 0, ly = 0, rx = 0, ry = 0;  // -1..1, y positivo = abajo
    bool a = false, y = false, start = false;
    bool lb = false, rb = false, zl = false, zr = false;
    bool up = false, down = false;
};

class Pad {
public:
    void refresh() {
        if (isOpen() && !attached()) close();
        if (!isOpen()) openFirst();
    }
    void close() {
#ifdef __SWITCH__
        if (joy_) SDL_JoystickClose(joy_);
        joy_ = nullptr;
#else
        if (pad_) SDL_GameControllerClose(pad_);
        pad_ = nullptr;
#endif
    }

    PadState poll() const {
        PadState s;
        if (!isOpen()) return s;
#ifdef __SWITCH__
        // Indices de libnx: botones 0=A 1=B 2=X 3=Y 6=L 7=R 8=ZL 9=ZR 10=+ 11=-
        // 12=izq 13=arriba 14=der 15=abajo. Ejes 0=LX 1=LY 2=RX 3=RY.
        s.lx = axis(SDL_JoystickGetAxis(joy_, 0));
        s.ly = axis(SDL_JoystickGetAxis(joy_, 1));
        s.rx = axis(SDL_JoystickGetAxis(joy_, 2));
        s.ry = axis(SDL_JoystickGetAxis(joy_, 3));
        s.a = SDL_JoystickGetButton(joy_, 0) != 0;
        s.y = SDL_JoystickGetButton(joy_, 3) != 0;
        s.lb = SDL_JoystickGetButton(joy_, 6) != 0;
        s.rb = SDL_JoystickGetButton(joy_, 7) != 0;
        s.zl = SDL_JoystickGetButton(joy_, 8) != 0;
        s.zr = SDL_JoystickGetButton(joy_, 9) != 0;
        s.start = SDL_JoystickGetButton(joy_, 10) != 0;
        s.up = SDL_JoystickGetButton(joy_, 13) != 0;
        s.down = SDL_JoystickGetButton(joy_, 15) != 0;
#else
        s.lx = axis(SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_LEFTX));
        s.ly = axis(SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_LEFTY));
        s.rx = axis(SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_RIGHTX));
        s.ry = axis(SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_RIGHTY));
        s.a = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_A) != 0;
        s.y = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_Y) != 0;
        s.lb = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;
        s.rb = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;
        s.zl = SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000;
        s.zr = SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000;
        s.start = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_START) != 0;
        s.up = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
        s.down = SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
#endif
        return s;
    }

private:
    static float axis(int raw) {
        float v = static_cast<float>(raw) / 32767.0f;
        const float dead = 0.18f;
        if (std::fabs(v) < dead) return 0.0f;
        v = (v > 0 ? v - dead : v + dead) / (1.0f - dead);
        return wm::clampf(v, -1.0f, 1.0f);
    }

#ifdef __SWITCH__
    bool isOpen() const { return joy_ != nullptr; }
    bool attached() const { return SDL_JoystickGetAttached(joy_) == SDL_TRUE; }
    void openFirst() {
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            joy_ = SDL_JoystickOpen(i);
            if (joy_) return;
        }
    }
    SDL_Joystick* joy_ = nullptr;
#else
    bool isOpen() const { return pad_ != nullptr; }
    bool attached() const { return SDL_GameControllerGetAttached(pad_) == SDL_TRUE; }
    void openFirst() {
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (!SDL_IsGameController(i)) continue;
            pad_ = SDL_GameControllerOpen(i);
            if (pad_) return;
        }
    }
    SDL_GameController* pad_ = nullptr;
#endif
};

void fatal(const char* msg) {
    std::fprintf(stderr, "ERROR: %s\n", msg);
#ifndef __SWITCH__
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Wither Sim", msg, nullptr);
#endif
}

}  // namespace

int main(int /*argc*/, char** /*argv*/) {
#if defined(__SWITCH__) && defined(WM_NXLINK)
    // Con 'nxlink -s' los printf/fprintf salen por la red hacia el PC.
    socketInitializeDefault();
    nxlinkStdio();
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0) {
        fatal(SDL_GetError());
        return 1;
    }

    // OpenGL 3.3 core: la version comun a Windows/Linux/macOS (macOS da 4.1 core)
    // y a Switch (Mesa/nouveau da 4.3 core).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef __SWITCH__
    const Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
#else
    const Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#endif
    SDL_Window* window = SDL_CreateWindow("Wither Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
                                          windowFlags);
    if (!window) {
        fatal(SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        fatal(SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);  // vsync

    wm::Renderer renderer;
    std::string error;
    if (!renderer.init(&SDL_GL_GetProcAddress, error)) {
        fatal(error.c_str());
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    wm::Game game;
    Pad pad;
    pad.refresh();
    PadState prevPad;

    bool running = true;
    bool fullscreen = false;
    Uint64 last = SDL_GetPerformanceCounter();
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());

    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(static_cast<double>(now - last) / freq);
        last = now;

        wm::InputState in;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (e.key.repeat) break;
                    if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                    if (e.key.keysym.sym == SDLK_TAB) in.toggleAuto = true;
                    if (e.key.keysym.sym == SDLK_r) in.resetPressed = true;
                    if (e.key.keysym.sym == SDLK_F11) {
                        fullscreen = !fullscreen;
                        SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    }
                    break;
                case SDL_MOUSEMOTION:
                    // Arrastrar con cualquier boton gira la camara.
                    if (e.motion.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK)) {
                        in.orbitDelta.x += -static_cast<float>(e.motion.xrel) * 0.006f;
                        in.orbitDelta.y += static_cast<float>(e.motion.yrel) * 0.006f;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    in.zoomDelta += -static_cast<float>(e.wheel.y);
                    break;
                case SDL_CONTROLLERDEVICEADDED:
                case SDL_CONTROLLERDEVICEREMOVED:
                case SDL_JOYDEVICEADDED:
                case SDL_JOYDEVICEREMOVED:
                    pad.refresh();
                    break;
                default:
                    break;
            }
        }

        // --- Teclado (estado continuo) ---
        const Uint8* k = SDL_GetKeyboardState(nullptr);
        auto key = [&](SDL_Scancode s) { return k[s] ? 1.0f : 0.0f; };
        in.move.x = key(SDL_SCANCODE_D) - key(SDL_SCANCODE_A);
        in.move.y = key(SDL_SCANCODE_W) - key(SDL_SCANCODE_S);
        in.rise = key(SDL_SCANCODE_E) - key(SDL_SCANCODE_Q);
        in.orbit.x = key(SDL_SCANCODE_RIGHT) - key(SDL_SCANCODE_LEFT);
        in.orbit.y = key(SDL_SCANCODE_DOWN) - key(SDL_SCANCODE_UP);
        in.zoom = key(SDL_SCANCODE_X) - key(SDL_SCANCODE_Z);

        // --- Mando ---
        PadState p = pad.poll();
        in.move.x += p.lx;
        in.move.y += -p.ly;                      // stick arriba = alejarse de la camara
        in.orbit.x += p.rx;
        in.orbit.y += p.ry;
        in.rise += (p.rb ? 1.0f : 0.0f) - (p.lb ? 1.0f : 0.0f);
        in.zoom += (p.zr || p.down ? 1.0f : 0.0f) - (p.zl || p.up ? 1.0f : 0.0f);
        if (p.a && !prevPad.a) in.toggleAuto = true;
        if (p.y && !prevPad.y) in.resetPressed = true;
        if (p.start && !prevPad.start) running = false;  // boton +
        prevPad = p;

        in.move.x = wm::clampf(in.move.x, -1.0f, 1.0f);
        in.move.y = wm::clampf(in.move.y, -1.0f, 1.0f);
        in.rise = wm::clampf(in.rise, -1.0f, 1.0f);
        in.orbit.x = wm::clampf(in.orbit.x, -1.0f, 1.0f);
        in.orbit.y = wm::clampf(in.orbit.y, -1.0f, 1.0f);
        in.zoom = wm::clampf(in.zoom, -1.0f, 1.0f);

        game.update(dt, in);

        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window, &w, &h);
        renderer.render(game, w, h);
        SDL_GL_SwapWindow(window);
    }

    renderer.shutdown();  // antes de destruir el contexto
    pad.close();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
#if defined(__SWITCH__) && defined(WM_NXLINK)
    socketExit();
#endif
    return 0;
}
