// Cargador minimo de OpenGL 3.3 core (reemplaza a glad para no depender de
// nada externo). Todas las funciones, incluidas las de GL 1.x, se piden con el
// mismo "getProcAddress" (SDL_GL_GetProcAddress en el juego), asi funciona igual
// en Windows, Linux, macOS y Switch sin enlazar libGL/opengl32.
#pragma once
#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
#define WM_GLAPI __stdcall
#else
#define WM_GLAPI
#endif

namespace gl {

using GLenum = unsigned int;
using GLbitfield = unsigned int;
using GLuint = unsigned int;
using GLint = int;
using GLsizei = int;
using GLfloat = float;
using GLboolean = unsigned char;
using GLchar = char;
using GLsizeiptr = std::ptrdiff_t;

constexpr GLenum COLOR_BUFFER_BIT = 0x4000, DEPTH_BUFFER_BIT = 0x0100;
constexpr GLenum DEPTH_TEST = 0x0B71, CULL_FACE = 0x0B44, BACK = 0x0405, CCW = 0x0901, LESS = 0x0201;
constexpr GLenum TRIANGLES = 0x0004, UNSIGNED_SHORT = 0x1403, UNSIGNED_BYTE = 0x1401, FLOAT = 0x1406;
constexpr GLenum ARRAY_BUFFER = 0x8892, ELEMENT_ARRAY_BUFFER = 0x8893, STATIC_DRAW = 0x88E4;
constexpr GLenum VERTEX_SHADER = 0x8B31, FRAGMENT_SHADER = 0x8B30;
constexpr GLenum COMPILE_STATUS = 0x8B81, LINK_STATUS = 0x8B82, INFO_LOG_LENGTH = 0x8B84;
constexpr GLenum TEXTURE_2D = 0x0DE1, TEXTURE0 = 0x84C0;
constexpr GLenum TEXTURE_MAG_FILTER = 0x2800, TEXTURE_MIN_FILTER = 0x2801;
constexpr GLenum TEXTURE_WRAP_S = 0x2802, TEXTURE_WRAP_T = 0x2803;
constexpr GLenum NEAREST = 0x2600, CLAMP_TO_EDGE = 0x812F, RGBA = 0x1908, RGBA8 = 0x8058;
constexpr GLenum UNPACK_ALIGNMENT = 0x0CF5, PACK_ALIGNMENT = 0x0D05;
constexpr GLenum VENDOR = 0x1F00, RENDERER = 0x1F01, VERSION = 0x1F02;
constexpr GLenum NO_ERROR_ = 0;

#define WM_GL_FUNCS(X)                                                                          \
    X(void, Clear, (GLbitfield mask))                                                           \
    X(void, ClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))                           \
    X(void, Viewport, (GLint x, GLint y, GLsizei w, GLsizei h))                                 \
    X(void, Enable, (GLenum cap))                                                               \
    X(void, Disable, (GLenum cap))                                                              \
    X(void, CullFace, (GLenum mode))                                                            \
    X(void, FrontFace, (GLenum mode))                                                           \
    X(void, DepthFunc, (GLenum func))                                                           \
    X(GLenum, GetError, (void))                                                                 \
    X(const unsigned char*, GetString, (GLenum name))                                           \
    X(void, PixelStorei, (GLenum pname, GLint param))                                           \
    X(void, ReadPixels, (GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* data)) \
    X(void, GenTextures, (GLsizei n, GLuint* textures))                                         \
    X(void, DeleteTextures, (GLsizei n, const GLuint* textures))                                \
    X(void, BindTexture, (GLenum target, GLuint texture))                                       \
    X(void, TexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei w, GLsizei h, GLint border, GLenum format, GLenum type, const void* pixels)) \
    X(void, TexParameteri, (GLenum target, GLenum pname, GLint param))                          \
    X(void, ActiveTexture, (GLenum texture))                                                    \
    X(void, GenVertexArrays, (GLsizei n, GLuint* arrays))                                       \
    X(void, BindVertexArray, (GLuint array))                                                    \
    X(void, DeleteVertexArrays, (GLsizei n, const GLuint* arrays))                              \
    X(void, GenBuffers, (GLsizei n, GLuint* buffers))                                           \
    X(void, BindBuffer, (GLenum target, GLuint buffer))                                         \
    X(void, BufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage))       \
    X(void, DeleteBuffers, (GLsizei n, const GLuint* buffers))                                  \
    X(void, EnableVertexAttribArray, (GLuint index))                                            \
    X(void, VertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)) \
    X(GLuint, CreateShader, (GLenum type))                                                      \
    X(void, ShaderSource, (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)) \
    X(void, CompileShader, (GLuint shader))                                                     \
    X(void, GetShaderiv, (GLuint shader, GLenum pname, GLint* params))                          \
    X(void, GetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
    X(void, DeleteShader, (GLuint shader))                                                      \
    X(GLuint, CreateProgram, (void))                                                            \
    X(void, AttachShader, (GLuint program, GLuint shader))                                      \
    X(void, LinkProgram, (GLuint program))                                                      \
    X(void, GetProgramiv, (GLuint program, GLenum pname, GLint* params))                        \
    X(void, GetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
    X(void, DeleteProgram, (GLuint program))                                                    \
    X(void, UseProgram, (GLuint program))                                                       \
    X(GLint, GetUniformLocation, (GLuint program, const GLchar* name))                          \
    X(void, UniformMatrix4fv, (GLint loc, GLsizei count, GLboolean transpose, const GLfloat* value)) \
    X(void, Uniform1i, (GLint loc, GLint v0))                                                   \
    X(void, Uniform3f, (GLint loc, GLfloat v0, GLfloat v1, GLfloat v2))                         \
    X(void, DrawElements, (GLenum mode, GLsizei count, GLenum type, const void* indices))

#define WM_GL_DECLARE(ret, name, args) extern ret(WM_GLAPI* name) args;
WM_GL_FUNCS(WM_GL_DECLARE)
#undef WM_GL_DECLARE

using GetProcFn = void* (*)(const char*);

// Carga todas las funciones. Devuelve false (y rellena 'missing' con el nombre
// de la primera que falte) si el contexto no es lo bastante nuevo.
bool load(GetProcFn getProc, const char** missing = nullptr);

}  // namespace gl
