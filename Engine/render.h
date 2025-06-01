#pragma once
// MSAA配置


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include "input_manager.h"
#include "gui_renderer.h"
#include "scene.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int width, int height, const char* title);
    void render(float time);
    void cleanup();
    bool shouldClose() const;

private:
    GLFWwindow* window;
    GLuint shaderProgram;
    // VAO: 顶点数组对象，用于存储顶点属性的配置
    // VBO: 顶点缓冲对象，用于存储顶点数据
    // EBO: 元素缓冲对象，用于存储顶点索引
    GLuint VAO, VBO, EBO;
    
    InputManager inputManager;
    GUIRenderer guiRenderer;
    Scene scene;
    Shader PBR_shader;

    void setupPBRShader();
    void loadTestRoom();

    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // 相机控制变量
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;

    // 帧率控制变量
    double lastFrameTime;
    double targetFrameTime;
    const int targetFPS = 60;
    
    // 帧率控制方法
    void controlFrameRate();

    // 窗口尺寸
    int windowWidth;
    int windowHeight;
    
    // 窗口大小变化回调
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static Renderer* currentInstance;
};