#pragma once
#include <vector>
#include <memory>
#include <string>
#include <glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"

class Model;

class Scene {
public:
    Scene();
    ~Scene();

    // 加载模型
    bool loadModel(const std::string& path);
    // 渲染场景
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    // 更新场景
    void update(float deltaTime);
    // 创建PBR材质
    void createPBRMaterial(Shader& shader);
    // PBR光照参数
    glm::vec3 lightPos{5.0f, 5.0f, 5.0f};
    glm::vec3 lightColor{300.0f, 300.0f, 300.0f};
    float lightIntensity{1.0f};
    std::vector<std::unique_ptr<Model>> models;
};