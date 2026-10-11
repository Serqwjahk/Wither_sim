cmake_minimum_required(VERSION 3.16)
project(WitherSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Unica dependencia externa: SDL2.
#   Linux:   sudo apt install libsdl2-dev        (o dnf/pacman equivalente)
#   macOS:   brew install sdl2
#   Windows: vcpkg install sdl2  (y -DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake)
#            o descargar el SDL2-devel de libsdl.org y pasar -DSDL2_DIR=<ruta>/cmake
find_package(SDL2 REQUIRED)

set(WITHER_CORE_SOURCES
  src/core/Game.cpp
  src/core/WitherModel.cpp
)
set(WITHER_RENDER_SOURCES
  src/render/Renderer.cpp
  src/render/gl_loader.cpp
)

add_executable(wither-sim
  ${WITHER_CORE_SOURCES}
  ${WITHER_RENDER_SOURCES}
  src/platform/main_sdl.cpp
)

if(TARGET SDL2::SDL2)
  target_link_libraries(wither-sim PRIVATE SDL2::SDL2)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(wither-sim PRIVATE SDL2::SDL2main)  # necesario en Windows
  endif()
else()
  # SDL2 antiguo sin targets importados
  target_include_directories(wither-sim PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(wither-sim PRIVATE ${SDL2_LIBRARIES})
endif()

if(MSVC)
  target_compile_options(wither-sim PRIVATE /W3)
else()
  target_compile_options(wither-sim PRIVATE -Wall -Wextra)
endif()

# Prueba sin ventana (solo Linux, necesita EGL + Mesa). Renderiza un PPM.
#   cmake -B build -DWITHER_OFFSCREEN_TEST=ON && cmake --build build
#   EGL_PLATFORM=surfaceless ./build/offscreen_test salida.ppm 3 1
option(WITHER_OFFSCREEN_TEST "Compilar tests/offscreen_test (Linux + EGL)" OFF)
if(WITHER_OFFSCREEN_TEST AND UNIX AND NOT APPLE)
  add_executable(offscreen_test
    tests/offscreen_test.cpp ${WITHER_CORE_SOURCES} ${WITHER_RENDER_SOURCES})
  target_link_libraries(offscreen_test PRIVATE EGL)
endif()
