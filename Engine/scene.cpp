#include "scene.h"
#include "model.h"
#include <iostream>
#include <filesystem>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
Scene::Scene() : 
    lightPos(5.0f, 5.0f, 5.0f),
    lightColor(300.0f, 300.0f, 300.0f),  // PBR需要更高的光照强度
    lightIntensity(1.0f) {
    spdlog::info("Scene: 开始初始化");
}

Scene::~Scene() {}

bool Scene::loadModel(const std::string& path) {
    spdlog::info("Scene: 开始加载模型文件 {}", path);
    try {
        spdlog::debug("Scene: 创建模型实例");
        // 使用绝对路径
        std::string fullPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Assets/Models/" + path;
        spdlog::info("开始加载模型: {}", fullPath);
        
        // 纹理贴图路径
        std::string albedoPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Assets/Textures/毛发_albedo.png";
        std::string normalPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Assets/Textures/毛发_normal.png";
        
        auto model = std::make_unique<Model>(fullPath.c_str());
        // 设置PBR纹理
        model->setTexturePaths(albedoPath, normalPath);
        models.push_back(std::move(model));
        spdlog::info("Scene: 模型加载完成");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Assimp加载模型失败: {}", e.what());
        return false;
    }
}

void Scene::render(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    // 更新光源位置，可以根据需要修改
    //lightPos = glm::vec3(5.0f * sin(glfwGetTime()), 5.0f, 5.0f * cos(glfwGetTime()));

    // 设置PBR光照参数
    shader.setVec3("lightPositions[0]", lightPos);
    shader.setVec3("lightColors[0]", lightColor * lightIntensity);
    shader.setVec3("camPos", glm::vec3(view[3]));
    shader.setFloat("lightIntensity", lightIntensity);

    // 设置变换矩阵
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // 渲染所有模型
    for (const auto& model : models) {
        // 传递一个默认的单位模型矩阵
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        
        // 设置PBR材质参数
        // 注意：这里假设模型只有一个网格，如果模型有多个网格，需要遍历网格并设置每个网格的材质
        // 更合适的做法是在 Model::draw 或 Mesh::draw 中设置材质 uniform
        // 但为了快速测试，暂时在这里设置第一个网格的材质
        if (!model->meshes.empty()) {
            shader.setVec3("albedoColor", model->meshes[0].material.basecolor);
            shader.setFloat("ao", model->meshes[0].material.ao);
            // 如果需要，也可以在这里设置 metallic 和 roughness 的默认值
            shader.setFloat("metallic", model->meshes[0].material.metallic);
            shader.setFloat("roughness", model->meshes[0].material.roughness);
        }

        model->draw(shader, modelMatrix, view, projection);
    }
}

void Scene::update(float deltaTime) {
    // 更新场景中的动态数据
    // 目前没有动态更新数据，可以根据需要添加
}