FILE(GLOB IMGUI_SOURCES *.cpp)
FILE(GLOB IMGUI_HEADERS *.h)
SET(BACKEND backends/imgui_impl_opengl3.cpp backends/imgui_impl_sdl.cpp)

ADD_LIBRARY(imgui_static STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS} ${BACKEND})
TARGET_INCLUDE_DIRECTORIES(imgui_static PUBLIC ./ ../utils ${SDL2_INCLUDE_DIRS})