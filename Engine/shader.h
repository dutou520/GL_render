#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm.hpp>

class Shader {
    //构造函数
public:
    // 构造函数读取并构建着色器
    Shader();

    // 着色器程序ID
    unsigned int ID;

    // 构造函数读取并构建着色器
    Shader(const char* vertexPath, const char* fragmentPath);
    // 从现有的着色器程序构造
    Shader(GLuint programId);

    // 使用/激活程序
    void use();

    // uniform工具函数
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;

private:
    // 检查着色器编译/链接错误的工具函数
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif