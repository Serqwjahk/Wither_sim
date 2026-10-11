#include "gl_loader.h"

namespace gl {

#define WM_GL_DEFINE(ret, name, args) ret(WM_GLAPI* name) args = nullptr;
WM_GL_FUNCS(WM_GL_DEFINE)
#undef WM_GL_DEFINE

bool load(GetProcFn getProc, const char** missing) {
    bool ok = true;
    const char* firstMissing = nullptr;
#define WM_GL_LOAD(ret, name, args)                                         \
    name = reinterpret_cast<decltype(name)>(getProc("gl" #name));           \
    if (!name && ok) {                                                      \
        ok = false;                                                         \
        firstMissing = "gl" #name;                                          \
    }
    WM_GL_FUNCS(WM_GL_LOAD)
#undef WM_GL_LOAD
    if (missing) *missing = firstMissing;
    return ok;
}

}  // namespace gl
