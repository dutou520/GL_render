#pragma once
#include <vector>
#include <string>
#include <glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"

// 顶点结构体，包含位置、法线和纹理坐标
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

// 纹理结构体，包含纹理ID、类型和路径
struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

// 材质结构体，包含环境光、漫反射、镜面反射等属性
struct PBR_Material {
    glm::vec3 basecolor;
    float metallic;
    float roughness;
    float ao; // 将 ao 的类型改为 float
    glm::vec3 emissionColor; // emissionColor 保持 glm::vec3

    bool useAlbedoMap;//反照率贴图
    bool useNormalMap;//法线贴图
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAOMap;
    bool useEmissionMap;

    Texture albedoMap;    // 漫反射贴图
    Texture normalMap;     // 法线贴图
    Texture heightMap;     // 高度贴图
    Texture roughnessMap;     // 粗糙度贴图
    Texture metallicMap;     // 金属度贴图
    Texture aoMap;     // 环境光遮蔽贴图
    Texture emissionMap;     // 自发光贴图
};

// 网格结构体，包含顶点数据、索引和材质
struct Mesh {
    std::vector<Vertex> vertices;        // 顶点数组
    std::vector<unsigned int> indices;   // 索引数组
    PBR_Material material;                   // 材质
    GLuint VAO, VBO, EBO;               // OpenGL缓冲对象

    void setupMesh();                    // 设置网格数据
    void draw(Shader& shader, const glm::mat4& modelMatrix, const glm::mat4& view, const glm::mat4& projection);           // 绘制网格
    void setupMaterial();  // 设置材质
private:
    void setupTextures(Shader& shader);  // 设置纹理
};

// 3D模型类
class Model {
public:
    Model(const char* path);             // 从文件加载模型
    void draw(Shader& shader, const glm::mat4& modelMatrix, const glm::mat4& view, const glm::mat4& projection);           // 绘制模型
    void setTexturePaths(const std::string& albedoPath, const std::string& normalPath); // 设置纹理路径
    std::vector<Mesh> meshes;           // 网格数组
private:
    
    std::string directory;              // 模型文件目录
    std::vector<Texture> textures_loaded; // 已加载的纹理
    std::string albedoTexturePath;      // 反照率贴图路径
    std::string normalTexturePath;      // 法线贴图路径

    void loadModel(const std::string& path);    // 加载模型
    void processNode(aiNode* node, const aiScene* scene);  // 处理节点
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);  // 处理网格
    PBR_Material loadMaterial(aiMaterial* mat);     // 加载材质
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);  // 加载材质纹理
    unsigned int TextureFromFile(const char* path, const std::string& directory);  // 从文件加载纹理
};