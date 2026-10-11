// Render OpenGL 3.3 core. Depende de core/ pero core/ no depende de el.
#pragma once
#include <string>
#include <vector>

#include "../core/Game.h"
#include "gl_loader.h"

namespace wm {

class Renderer {
public:
    ~Renderer() { shutdown(); }

    // Hay que llamarlo con el contexto GL ya activo.
    bool init(gl::GetProcFn getProc, std::string& error);
    void shutdown();

    // fbWidth/fbHeight en pixeles reales del framebuffer.
    void render(const Game& game, int fbWidth, int fbHeight);

private:
    struct Mesh {
        gl::GLuint vao = 0, vbo = 0, ebo = 0;
        gl::GLsizei indexCount = 0;
    };

    Mesh upload(const std::vector<Vertex>& v, const std::vector<uint16_t>& idx);
    void destroy(Mesh& m);
    void draw(const Mesh& m);
    bool buildProgram(std::string& error);

    gl::GLuint program_ = 0, texture_ = 0;
    gl::GLint uModel_ = -1, uViewProj_ = -1, uMode_ = -1, uColor_ = -1, uTex_ = -1;
    std::vector<Mesh> partMeshes_;  // una por parte del wither
    Mesh cube_, ground_;
    bool ready_ = false;
};

}  // namespace wm
