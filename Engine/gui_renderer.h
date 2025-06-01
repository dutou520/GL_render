#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <vector>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "scene.h"
// GUI渲染器类，用于渲染坐标轴和网格
class GUIRenderer {
public:
    // 球体着色器参数
    struct LightingParams {
        float lightPos[3] = {5.0f, 5.0f, 5.0f};
        float lightColor[3] = {1.0f, 1.0f, 1.0f};
        float ambientStrength = 0.1f;
        float diffuseStrength = 0.8f;
        float specularStrength = 0.5f;
    };

    // 构造函数和析构函数
    GUIRenderer();
    ~GUIRenderer();

    // 初始化GUI渲染器
    void init();
    // 渲染三维坐标轴
    void renderAxis();
    // 渲染参考网格
    void renderGrid();
    // 清理所有OpenGL资源
    void cleanup();
    // 设置投影矩阵
    void setProjectionMatrix(const glm::mat4& projection);
    // 设置视图矩阵
    void setViewMatrix(const glm::mat4& view);

    // ImGui相关方法
    void initImGui(GLFWwindow* window);
    void renderImGui(Scene& scene);
    void cleanupImGui();
    void renderLightingControls(Scene& scene);
    
    // 获取球体着色器参数
    const LightingParams& getLightingParams() const { return lightingParams; }

private:
    // 设置坐标轴的顶点数据和缓冲区
    void setupAxis();
    // 设置网格的顶点数据和缓冲区
    void setupGrid();

    // OpenGL顶点数组对象和顶点缓冲区对象（坐标轴）
    GLuint axisVAO, axisVBO;
    // OpenGL顶点数组对象和顶点缓冲区对象（网格）
    GLuint gridVAO, gridVBO;
    // 着色器程序句柄
    GLuint GUI_shaderProgram;

    LightingParams lightingParams;
};