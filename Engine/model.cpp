#include "model.h"
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <fstream>
// 模型构造函数
Model::Model(const char* path) {
    loadModel(path);
}

// 绘制模型
void Model::draw(Shader& shader, const glm::mat4& modelMatrix, const glm::mat4& view, const glm::mat4& projection) {
    for (auto& mesh : meshes) {
        mesh.draw(shader, modelMatrix, view, projection);
    }
}

// 加载模型
void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        spdlog::error("Assimp加载模型失败: {}", importer.GetErrorString());
        return;
    }
    
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

// 处理节点
void Model::processNode(aiNode* node, const aiScene* scene) {
    // 处理节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

// 处理网格
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    Mesh result;

    // 处理顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }
        result.vertices.push_back(vertex);
    }
    spdlog::info("处理顶点数据完成，共 {} 个顶点", mesh->mNumVertices);
    if (!result.vertices.empty()) {
        spdlog::debug("第一个顶点位置: ({}, {}, {})", 
            result.vertices[0].position.x,
            result.vertices[0].position.y,
            result.vertices[0].position.z);
    }

    // 处理索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            result.indices.push_back(face.mIndices[j]);
        }
    }
    spdlog::info("处理索引数据完成，共 {} 个三角形", mesh->mNumFaces);
    if (!result.indices.empty()) {
        spdlog::debug("第一个三角形索引: {}, {}, {}",
            result.indices[0],
            result.indices[1],
            result.indices[2]);
    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        result.material = loadMaterial(material);
    }

    result.setupMesh();
    return result;
}

// 从文件加载纹理
unsigned int Model::TextureFromFile(const char* path, const std::string& directory) {
    std::string filename = directory + '/' + std::string(path);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1) ? GL_RED : 
                       (nrComponents == 3) ? GL_RGB : GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        spdlog::error("Texture failed to load at path: {}", path);
        stbi_image_free(data);
    }
    spdlog::info("Texture loaded at path: {}", path);
    return textureID;
}

// 加载材质纹理
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

// 加载材质，暂时用不上
void Model::setTexturePaths(const std::string& albedoPath, const std::string& normalPath) {
    albedoTexturePath = albedoPath;
    normalTexturePath = normalPath;
}

PBR_Material Model::loadMaterial(aiMaterial* mat) {
    PBR_Material material;
    
    // 设置默认PBR参数
    material.basecolor = glm::vec3(1.0f);
    material.metallic = 0.0f;
    material.roughness = 0.5f;
    material.ao = 1.0f; // Default AO to 1.0 (no occlusion)
    material.emissionColor = glm::vec3(0.0f); // Default emission to 0.0
    
    // 加载纹理
    // Assimp uses different texture types, map them to PBR types
    std::vector<Texture> albedoMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "albedo");
    if (!albedoMaps.empty()) material.albedoMap = albedoMaps[0];

    std::vector<Texture> normalMaps = loadMaterialTextures(mat, aiTextureType_NORMALS, "normal");
    if (!normalMaps.empty()) material.normalMap = normalMaps[0];

    std::vector<Texture> metallicMaps = loadMaterialTextures(mat, aiTextureType_METALNESS, "metallic");
    if (!metallicMaps.empty()) material.metallicMap = metallicMaps[0];

    std::vector<Texture> roughnessMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness");
    if (!roughnessMaps.empty()) material.roughnessMap = roughnessMaps[0];

    std::vector<Texture> aoMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT_OCCLUSION, "ao");
    if (!aoMaps.empty()) material.aoMap = aoMaps[0];

    std::vector<Texture> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, "emission");
    if (!emissionMaps.empty()) material.emissionMap = emissionMaps[0];
    
    return material;
}

// 设置网格
void Mesh::setupMesh() {
    // 生成VAO和缓冲区
    glGenVertexArrays(1, &VAO);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("setupMesh - OpenGL错误(生成VAO): {:#x}", error);
        return;
    }

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("setupMesh - OpenGL错误(生成缓冲区): {:#x}", error);
        return;
    }

    // 检查VAO是否有效
    if (VAO == 0) {
        spdlog::error("setupMesh - VAO生成失败");
        return;
    }

    // 检查VBO和EBO是否有效
    if (VBO == 0 || EBO == 0) {
        spdlog::error("setupMesh - VBO或EBO生成失败");
        return;
    }

    // 检查顶点数据和索引数据是否为空
    if (vertices.empty() || indices.empty()) {
        spdlog::error("setupMesh - 顶点数据或索引数据为空");
        return;
    }

    // 绑定VAO
    glBindVertexArray(VAO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("setupMesh - OpenGL错误(绑定VAO): {:#x}", error);
        return;
    }
    
    // 设置顶点缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("setupMesh - OpenGL错误(设置顶点缓冲区): {:#x}", error);
        return;
    }

    // 设置索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("setupMesh - OpenGL错误(设置索引缓冲区): {:#x}", error);
        return;
    }

    // 设置顶点属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("setupMesh - OpenGL错误(设置顶点属性): {:#x}", error);
        return;
    }

    // 解绑VAO
    glBindVertexArray(0);
    spdlog::debug("setupMesh - 成功设置网格数据，VAO ID: {}", VAO);
}

// 设置纹理
void Mesh::setupTextures(Shader& shader) {

}


//根据纹理是否存在设置材质
void Mesh::setupMaterial()
{
    material.useAlbedoMap = false;
    material.useNormalMap = false;
    material.useMetallicMap = false;
    material.useRoughnessMap = false;
    material.useAOMap = false;
    material.useEmissionMap = false;

    if(material.albedoMap.id != 0){
        material.useAlbedoMap = true;
    }
    if(material.normalMap.id!= 0){
        material.useNormalMap = true; 
    }
    if(material.metallicMap.id!= 0){
        material.useMetallicMap = true; 
    }
    if(material.roughnessMap.id!= 0){
        material.useRoughnessMap = true; 
    }
    if(material.aoMap.id!= 0){
       material.useAOMap = true; 
    }
    if(material.emissionMap.id!= 0){
        material.useEmissionMap = true; 
    }
}

// 绘制网格
void Mesh::draw(Shader& shader, const glm::mat4& modelMatrix, const glm::mat4& view, const glm::mat4& projection) {
    // 检查VAO是否有效
    if (VAO == 0) {
        spdlog::error("Mesh::draw - 无效的VAO");
        return;
    }

    // 检查着色器程序是否有效
    if (shader.ID == 0) {
        spdlog::error("Mesh::draw - 无效的着色器程序ID");
        return;
    }else{
        spdlog::debug("Mesh::draw - 使用着色器程序 ID: {}", shader.ID);
    }

    // 清除之前的OpenGL错误
    while (glGetError() != GL_NO_ERROR) {}
    //使用着色器
    shader.use();
    
    // 设置模型矩阵 - 这是关键的缺失部分！
    shader.setMat4("model", modelMatrix);
    
    // 设置视图和投影矩阵
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    // 设置材质
    setupMaterial();
    // 设置基础PBR材质参数
    shader.setVec3("material.basecolor", material.basecolor);
    shader.setFloat("material.metallic", material.metallic);
    shader.setFloat("material.roughness", material.roughness);

    // 设置贴图使用状态
    shader.setBool("material.useAlbedoMap", material.useAlbedoMap);
    shader.setBool("material.useNormalMap", material.useNormalMap);
    shader.setBool("material.useMetallicMap", material.useMetallicMap);
    shader.setBool("material.useRoughnessMap", material.useRoughnessMap);
    shader.setBool("material.useAOMap", material.useAOMap);
    shader.setBool("material.useEmissionMap", material.useEmissionMap);

    // 绑定贴图
    if (material.useAlbedoMap) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.albedoMap.id);
        shader.setInt("material.albedoMap", 0);
    }
    if (material.useNormalMap) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.normalMap.id);
        shader.setInt("material.normalMap", 1);
    }
    if (material.useMetallicMap) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.metallicMap.id);
        shader.setInt("material.metallicMap", 2);
    }
    if (material.useRoughnessMap) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, material.roughnessMap.id);
        shader.setInt("material.roughnessMap", 3);
    }
    if (material.useAOMap) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, material.aoMap.id);
        shader.setInt("material.aoMap", 4);
    }
    if (material.useEmissionMap) {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, material.emissionMap.id);
        shader.setInt("material.emissionMap", 5);
    }
    
    spdlog::debug("Mesh::draw - VAO ID: {}, 索引数量: {}", VAO, indices.size());
    
    // 绘制网格
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // 解绑所有贴图
    for (unsigned int i = 0; i < 6; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}