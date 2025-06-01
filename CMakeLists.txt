cmake_minimum_required(VERSION 3.10)

project(GL_Render VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW")
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glad_nullextensions")
set(GLM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm")
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ImGUI")
set(ASSIMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/assimp")

# 主可执行文件
add_executable(${PROJECT_NAME} main.cpp)

# 添加头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
    ${GLM_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb
    ${IMGUI_DIR}
    ${ASSIMP_DIR}/include
)

# 添加库文件目录
link_directories(
    ${GLFW_DIR}/lib-mingw-w64
    ${ASSIMP_DIR}/lib
)

# 添加子目录
add_subdirectory(Engine)
add_subdirectory(ThirdParty/ImGUI)
# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    engine
    ${GLFW_DIR}/lib-mingw-w64/libglfw3.a
    ${ASSIMP_DIR}/lib/libassimp.dll.a
    opengl32
)

# 复制assimp DLL到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_DIR}/lib/libassimp-5.dll"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libassimp-5.dll"
)
