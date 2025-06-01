# 首先单独编译 GLAD
project(glad LANGUAGES C)

add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/ThirdParty/glad_nullextensions/src/glad.c
)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

target_include_directories(glad PUBLIC
    ${GLAD_DIR}/include
)

# 设置C编译器标志
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置ImGui目录
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/ImGUI")

# 然后定义 engine 库并链接 glad
add_library(engine
    render.cpp
    render.h
    input_manager.cpp
    input_manager.h
    gui_renderer.cpp
    gui_renderer.h
    shader.cpp
    stb_impl.cpp
    model.cpp
    model.h
    scene.cpp
    scene.h
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(engine PUBLIC
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
    ${GLM_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/stb
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${ASSIMP_DIR}/include
)

target_link_libraries(engine PRIVATE
    glad
    ${CMAKE_SOURCE_DIR}/ThirdParty/GLFW/lib-mingw-w64/libglfw3.a
    ${ASSIMP_DIR}/lib/libassimp.dll.a
    opengl32
)

