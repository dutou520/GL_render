#include "input_manager.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <stb/stb_image_write.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#define SENSITIVITY 0.05f

InputManager::InputManager()
    : yaw(-90.0f), pitch(0.0f), lastX(400.0f), lastY(300.0f),
      firstMouse(true), cameraSpeed(2.5f * 0.016f),
      currentCameraFront(0.0f, 0.0f, -1.0f), cursorEnabled(false),
      graveKeyPressed(false) {}

void InputManager::init(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, staticMouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::processInput(GLFWwindow* window, glm::vec3& cameraPos,
                               const glm::vec3& cameraFront, const glm::vec3& cameraUp) {
    // ESC键退出程序
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // `键 - 切换鼠标显示状态
    if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
        if (!graveKeyPressed) {
            graveKeyPressed = true;
            cursorEnabled = !cursorEnabled;
            firstMouse = true;  // 重置firstMouse标志
            glfwSetInputMode(window, GLFW_CURSOR, 
                cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
    } else {
        graveKeyPressed = false;
    }

    // 只在鼠标隐藏状态下处理移动输入
    if (!cursorEnabled) {
        // W键 - 向前移动
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        
        // S键 - 向后移动
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        
        // A键 - 向左移动
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        
        // D键 - 向右移动
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        
        // E键 - 向上移动
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraUp;
        
        // Q键 - 向下移动
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraUp;
    }

    // P键 - 截图
    static bool pKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!pKeyPressed) {
            pKeyPressed = true;
            // 捕获截图
            captureScreenshot(window);
        }
    } else {
        pKeyPressed = false;
    }
}

void InputManager::mouseCallback(double xpos, double ypos) {
    // 只在鼠标隐藏状态下处理视角更新
    if (cursorEnabled) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = SENSITIVITY;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    currentCameraFront = glm::normalize(front);
}

void InputManager::staticMouseCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (inputManager) {
        inputManager->mouseCallback(xpos, ypos);
    }
}

void InputManager::captureScreenshot(GLFWwindow* window) {
    // 获取窗口大小
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    spdlog::info("the window:: width: {}, height: {}", width, height);

    // 分配内存存储像素数据
    unsigned char* pixels = new unsigned char[width * height * 3];
    unsigned char* flipped = new unsigned char[width * height * 3];

    // 保存当前渲染状态
    GLint prevFbo, prevReadFbo, prevDrawFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFbo);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFbo);

    // 确保像素数据对齐正确
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    // 绑定到默认帧缓冲区（屏幕）
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 设置读取后缓冲区
    glReadBuffer(GL_BACK);
    // 强制执行所有挂起的GL命令并等待完成
    glFlush();
    glFinish();
    // 额外等待以确保渲染完成
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 等待一小段时间确保渲染完成
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // 读取像素数据到缓冲区
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // 垂直翻转图像
    for (int y = 0; y < height; y++) {
        memcpy(&flipped[(height - 1 - y) * width * 3],
               &pixels[y * width * 3],
               width * 3);
    }

    // 生成带有时间戳的文件名
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "screenshot_" << std::put_time(std::localtime(&now_time), "%Y%m%d_%H%M%S") << ".png";
    std::string filename = ss.str();

    // 保存图像
    stbi_write_png(filename.c_str(), width, height, 3, flipped, width * 3);
    spdlog::info("Screenshot saved as: {}", filename);

    // 恢复之前的帧缓冲区状态
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

    // 释放内存
    delete[] pixels;
    delete[] flipped;
}