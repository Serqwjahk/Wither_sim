#include "Renderer.h"

#include <cstddef>

#include "wither_texture.h"

namespace wm {

static_assert(sizeof(Vertex) == 8 * sizeof(float), "Vertex debe ser pos(3) + normal(3) + uv(2) sin relleno");

namespace {

const char* kVertexShader = R"GLSL(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uViewProj;
out vec3 vNormal;
out vec2 vUV;
out vec3 vWorld;
void main() {
    vec4 w = uModel * vec4(aPos, 1.0);
    vWorld = w.xyz;
    vNormal = mat3(uModel) * aNormal;   // escala uniforme: basta con normalizar luego
    vUV = aUV;
    gl_Position = uViewProj * w;
}
)GLSL";

const char* kFragmentShader = R"GLSL(#version 330 core
in vec3 vNormal;
in vec2 vUV;
in vec3 vWorld;
out vec4 FragColor;
uniform sampler2D uTex;
uniform int uMode;      // 0 = modelo con textura, 1 = color plano (marcadores), 2 = suelo
uniform vec3 uColor;
const vec3 kLightDir = normalize(vec3(-0.4, -1.0, -0.6));  // hacia donde viaja la luz
void main() {
    vec3 n = normalize(vNormal);
    float light = 0.5 + 0.5 * max(dot(n, -kLightDir), 0.0);
    if (uMode == 0) {
        vec4 t = texture(uTex, vUV);
        if (t.a < 0.5) discard;
        FragColor = vec4(t.rgb * light, 1.0);
    } else if (uMode == 1) {
        FragColor = vec4(uColor * (0.7 + 0.3 * light), 1.0);
    } else {
        vec2 c = floor(vWorld.xz);
        float k = mod(c.x + c.y, 2.0);
        vec3 col = mix(vec3(0.13, 0.14, 0.17), vec3(0.19, 0.20, 0.24), k);
        float fade = clamp(length(vWorld.xz) / 60.0, 0.0, 1.0);
        FragColor = vec4(mix(col, vec3(0.05, 0.05, 0.08), fade), 1.0);
    }
}
)GLSL";

gl::GLuint compile(gl::GLenum type, const char* src, std::string& error) {
    gl::GLuint s = gl::CreateShader(type);
    gl::ShaderSource(s, 1, &src, nullptr);
    gl::CompileShader(s);
    gl::GLint ok = 0;
    gl::GetShaderiv(s, gl::COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024] = {};
        gl::GetShaderInfoLog(s, sizeof(log), nullptr, log);
        error = std::string(type == gl::VERTEX_SHADER ? "vertex shader: " : "fragment shader: ") + log;
        gl::DeleteShader(s);
        return 0;
    }
    return s;
}

}  // namespace

bool Renderer::buildProgram(std::string& error) {
    gl::GLuint vs = compile(gl::VERTEX_SHADER, kVertexShader, error);
    if (!vs) return false;
    gl::GLuint fs = compile(gl::FRAGMENT_SHADER, kFragmentShader, error);
    if (!fs) {
        gl::DeleteShader(vs);
        return false;
    }
    program_ = gl::CreateProgram();
    gl::AttachShader(program_, vs);
    gl::AttachShader(program_, fs);
    gl::LinkProgram(program_);
    gl::DeleteShader(vs);
    gl::DeleteShader(fs);
    gl::GLint ok = 0;
    gl::GetProgramiv(program_, gl::LINK_STATUS, &ok);
    if (!ok) {
        char log[1024] = {};
        gl::GetProgramInfoLog(program_, sizeof(log), nullptr, log);
        error = std::string("link: ") + log;
        return false;
    }
    uModel_ = gl::GetUniformLocation(program_, "uModel");
    uViewProj_ = gl::GetUniformLocation(program_, "uViewProj");
    uMode_ = gl::GetUniformLocation(program_, "uMode");
    uColor_ = gl::GetUniformLocation(program_, "uColor");
    uTex_ = gl::GetUniformLocation(program_, "uTex");
    return true;
}

Renderer::Mesh Renderer::upload(const std::vector<Vertex>& v, const std::vector<uint16_t>& idx) {
    Mesh m;
    m.indexCount = static_cast<gl::GLsizei>(idx.size());
    gl::GenVertexArrays(1, &m.vao);
    gl::GenBuffers(1, &m.vbo);
    gl::GenBuffers(1, &m.ebo);
    gl::BindVertexArray(m.vao);
    gl::BindBuffer(gl::ARRAY_BUFFER, m.vbo);
    gl::BufferData(gl::ARRAY_BUFFER, static_cast<gl::GLsizeiptr>(v.size() * sizeof(Vertex)), v.data(), gl::STATIC_DRAW);
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m.ebo);
    gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, static_cast<gl::GLsizeiptr>(idx.size() * sizeof(uint16_t)), idx.data(),
                   gl::STATIC_DRAW);
    const gl::GLsizei stride = sizeof(Vertex);
    gl::EnableVertexAttribArray(0);
    gl::VertexAttribPointer(0, 3, gl::FLOAT, 0, stride, reinterpret_cast<const void*>(0));
    gl::EnableVertexAttribArray(1);
    gl::VertexAttribPointer(1, 3, gl::FLOAT, 0, stride, reinterpret_cast<const void*>(3 * sizeof(float)));
    gl::EnableVertexAttribArray(2);
    gl::VertexAttribPointer(2, 2, gl::FLOAT, 0, stride, reinterpret_cast<const void*>(6 * sizeof(float)));
    gl::BindVertexArray(0);
    return m;
}

void Renderer::destroy(Mesh& m) {
    if (m.vao) gl::DeleteVertexArrays(1, &m.vao);
    if (m.vbo) gl::DeleteBuffers(1, &m.vbo);
    if (m.ebo) gl::DeleteBuffers(1, &m.ebo);
    m = Mesh();
}

void Renderer::draw(const Mesh& m) {
    gl::BindVertexArray(m.vao);
    gl::DrawElements(gl::TRIANGLES, m.indexCount, gl::UNSIGNED_SHORT, nullptr);
}

bool Renderer::init(gl::GetProcFn getProc, std::string& error) {
    const char* missing = nullptr;
    if (!gl::load(getProc, &missing)) {
        error = std::string("Falta la funcion OpenGL ") + (missing ? missing : "?") +
                " (se necesita un contexto 3.3 core)";
        return false;
    }
    if (!buildProgram(error)) return false;

    // Textura del wither (incrustada). NEAREST para conservar el look pixelado.
    gl::GenTextures(1, &texture_);
    gl::BindTexture(gl::TEXTURE_2D, texture_);
    gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA8, embedded::kWitherTexW, embedded::kWitherTexH, 0, gl::RGBA,
                   gl::UNSIGNED_BYTE, embedded::kWitherTexRGBA);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::NEAREST);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);

    // Una malla por parte del wither: se sube una vez, luego solo cambia uModel.
    WitherModel model;
    for (const Part& p : model.parts()) partMeshes_.push_back(upload(p.vertices, p.indices));

    // Cubo unitario centrado (marcadores de objetivo).
    {
        std::vector<Vertex> v;
        std::vector<uint16_t> idx;
        const Vec3 normals[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
        for (const Vec3& n : normals) {
            Vec3 a = std::fabs(n.y) > 0.5f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
            Vec3 u = normalize(cross(n, a));
            Vec3 w = cross(n, u);
            uint16_t base = static_cast<uint16_t>(v.size());
            const float sx[4] = {-1, 1, 1, -1}, sy[4] = {-1, -1, 1, 1};
            for (int i = 0; i < 4; ++i) {
                Vertex vt;
                vt.pos = (n + u * sx[i] + w * sy[i]) * 0.5f;
                vt.normal = n;
                v.push_back(vt);
            }
            // Orientar antihorario visto desde fuera.
            Vec3 c = cross(v[base + 1].pos - v[base].pos, v[base + 2].pos - v[base].pos);
            if (dot(c, n) >= 0) {
                for (uint16_t i : {0, 1, 2, 0, 2, 3}) idx.push_back(static_cast<uint16_t>(base + i));
            } else {
                for (uint16_t i : {0, 2, 1, 0, 3, 2}) idx.push_back(static_cast<uint16_t>(base + i));
            }
        }
        cube_ = upload(v, idx);
    }

    // Suelo: un quad grande en y = 0 (el patron de damero se calcula en el shader).
    {
        const float s = 100.0f;
        std::vector<Vertex> v(4);
        const Vec3 pts[4] = {{-s, 0, -s}, {-s, 0, s}, {s, 0, s}, {s, 0, -s}};
        for (int i = 0; i < 4; ++i) {
            v[i].pos = pts[i];
            v[i].normal = Vec3(0, 1, 0);
        }
        ground_ = upload(v, {0, 1, 2, 0, 2, 3});
    }

    ready_ = true;
    return true;
}

void Renderer::shutdown() {
    if (!ready_) return;
    for (Mesh& m : partMeshes_) destroy(m);
    partMeshes_.clear();
    destroy(cube_);
    destroy(ground_);
    if (texture_) gl::DeleteTextures(1, &texture_);
    if (program_) gl::DeleteProgram(program_);
    texture_ = program_ = 0;
    ready_ = false;
}

void Renderer::render(const Game& game, int fbWidth, int fbHeight) {
    if (!ready_ || fbWidth <= 0 || fbHeight <= 0) return;

    gl::Viewport(0, 0, fbWidth, fbHeight);
    gl::ClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
    gl::Enable(gl::DEPTH_TEST);
    gl::DepthFunc(gl::LESS);
    gl::Enable(gl::CULL_FACE);
    gl::CullFace(gl::BACK);
    gl::FrontFace(gl::CCW);

    const float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
    const Mat4 viewProj = perspective(radians(60.0f), aspect, 0.1f, 200.0f) * game.camera().view();

    gl::UseProgram(program_);
    gl::UniformMatrix4fv(uViewProj_, 1, 0, viewProj.data());
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, texture_);
    gl::Uniform1i(uTex_, 0);

    // Suelo
    gl::Uniform1i(uMode_, 2);
    gl::UniformMatrix4fv(uModel_, 1, 0, Mat4::identity().data());
    draw(ground_);

    // Wither
    gl::Uniform1i(uMode_, 0);
    const auto& parts = game.wither().parts();
    for (size_t i = 0; i < parts.size() && i < partMeshes_.size(); ++i) {
        gl::UniformMatrix4fv(uModel_, 1, 0, parts[i].world.data());
        draw(partMeshes_[i]);
    }

    // Marcadores de objetivo (cubos de color)
    gl::Uniform1i(uMode_, 1);
    for (int i = 0; i < 3; ++i) {
        const Marker& mk = game.markers()[i];
        Mat4 m = translate(mk.position) * scale(Vec3(0.35f, 0.35f, 0.35f));
        gl::UniformMatrix4fv(uModel_, 1, 0, m.data());
        gl::Uniform3f(uColor_, mk.color.x, mk.color.y, mk.color.z);
        draw(cube_);
    }
}

}  // namespace wm
