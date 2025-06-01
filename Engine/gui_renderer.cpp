#include "gui_renderer.h"
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
#include "shader.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "scene.h"
#include "model.h"
GUIRenderer::GUIRenderer() : axisVAO(0), axisVBO(0), gridVAO(0), gridVBO(0), GUI_shaderProgram(0) {}

GUIRenderer::~GUIRenderer() {
    cleanup();
}

void GUIRenderer::init() {
    // 创建并编译着色器程序
    const char* vertPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Shader/gui.vert";
    const char* fragPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Shader/gui.frag";
    try {
        Shader guiShader(vertPath, fragPath);
        GUI_shaderProgram = guiShader.ID;
    } catch (const std::exception& e) {
        std::cerr << "Error loading shader files: " << e.what() << std::endl;
        std::cerr << "Vertex shader path: " << vertPath << std::endl;
        std::cerr << "Fragment shader path: " << fragPath << std::endl;
        throw;
    }

    setupAxis();
    setupGrid();
}

void GUIRenderer::setupAxis() {


    // 生成并绑定顶点数组对象和顶点缓冲对象
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);

    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    // 将坐标轴顶点数据传输到GPU
    // 定义坐标轴顶点数据
    float axisVertices[] = {
        // 位置              // 颜色
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,  // X轴起点（红色）
        50.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,  // X轴终点
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,  // Y轴起点（绿色）
        0.0f, 50.0f, 0.0f,   0.0f, 1.0f, 0.0f,  // Y轴终点
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,  // Z轴起点（蓝色）
        0.0f, 0.0f, 50.0f,   0.0f, 0.0f, 1.0f   // Z轴终点
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    // 设置顶点属性指针 - 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 设置顶点属性指针 - 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void GUIRenderer::setupGrid() {
    std::vector<float> gridVertices;
    float size = 10.0f;
    float step = 1.0f;
    glm::vec3 gridColor(0.5f, 0.5f, 0.5f);

    for(float i = -size; i <= size; i += step) {
        if (i == 0) {
            // X方向的线（只生成负方向）
            gridVertices.push_back(0.0f); gridVertices.push_back(0.0f); gridVertices.push_back(-size);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);
            gridVertices.push_back(0.0f); gridVertices.push_back(0.0f); gridVertices.push_back(0.0f);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);

            // Z方向的线（只生成负方向）
            gridVertices.push_back(-size); gridVertices.push_back(0.0f); gridVertices.push_back(0.0f);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);
            gridVertices.push_back(0.0f); gridVertices.push_back(0.0f); gridVertices.push_back(0.0f);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);
        } else {
            // X方向的线
            gridVertices.push_back(i); gridVertices.push_back(0.0f); gridVertices.push_back(-size);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);
            gridVertices.push_back(i); gridVertices.push_back(0.0f); gridVertices.push_back(size);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);

            // Z方向的线
            gridVertices.push_back(-size); gridVertices.push_back(0.0f); gridVertices.push_back(i);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);
            gridVertices.push_back(size); gridVertices.push_back(0.0f); gridVertices.push_back(i);
            gridVertices.push_back(gridColor.x); gridVertices.push_back(gridColor.y); gridVertices.push_back(gridColor.z);
        }
    }

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void GUIRenderer::setProjectionMatrix(const glm::mat4& projection) {
    glUseProgram(GUI_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(GUI_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void GUIRenderer::setViewMatrix(const glm::mat4& view) {
    glUseProgram(GUI_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(GUI_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

void GUIRenderer::renderAxis() {
    glUseProgram(GUI_shaderProgram);
    glBindVertexArray(axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void GUIRenderer::renderGrid() {
    glUseProgram(GUI_shaderProgram);
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, 84);
    glBindVertexArray(0);
}

void GUIRenderer::cleanup() {
    if (axisVAO) glDeleteVertexArrays(1, &axisVAO);
    if (axisVBO) glDeleteBuffers(1, &axisVBO);
    if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
    if (gridVBO) glDeleteBuffers(1, &gridVBO);
    cleanupImGui();
}

void GUIRenderer::initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.Fonts->AddFontFromFileTTF("C:/Users/dutou/Documents/CppPrograms/GL_Render/fonts/msyh.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUIRenderer::cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUIRenderer::renderImGui(Scene& scene) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderLightingControls(scene);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIRenderer::renderLightingControls(Scene& scene) {
    //设置ImGUI窗口大小
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    //设置ImGUI窗口位置
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("光照控制面板");

    // 光源位置控制
    ImGui::Text("光源位置");
    if (ImGui::SliderFloat3("##LightPos", glm::value_ptr(scene.lightPos), -10.0f, 10.0f)) {
        std::copy(glm::value_ptr(scene.lightPos), glm::value_ptr(scene.lightPos) + 3, lightingParams.lightPos);
    }

    // 光源颜色控制
    ImGui::Text("光源颜色");
    if (ImGui::ColorEdit3("##LightColor", glm::value_ptr(scene.lightColor))) {
        std::copy(glm::value_ptr(scene.lightColor), glm::value_ptr(scene.lightColor) + 3, lightingParams.lightColor);
    }

    // PBR材质参数
    ImGui::Text("PBR材质参数");

    PBR_Material& material = scene.models[0]->meshes[0].material;
    if (ImGui::ColorEdit3("基础颜色", glm::value_ptr(material.basecolor))) {
        scene.models[0]->meshes[0].material.basecolor = material.basecolor;
    }
    if (ImGui::SliderFloat("金属度", &material.metallic, 0.0f, 1.0f)) {
        scene.models[0]->meshes[0].material.metallic = material.metallic;
    }
    if (ImGui::SliderFloat("粗糙度", &material.roughness, 0.0f, 1.0f)) {
        scene.models[0]->meshes[0].material.roughness = material.roughness;
    }


    ImGui::End();
}