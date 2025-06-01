#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <spdlog/spdlog.h>
class InputManager {
public:
    InputManager();
    
    void init(GLFWwindow* window);
    void processInput(GLFWwindow* window, glm::vec3& cameraPos, 
                     const glm::vec3& cameraFront, const glm::vec3& cameraUp);
    void mouseCallback(double xpos, double ypos);
    void captureScreenshot(GLFWwindow* window);
    
    static void staticMouseCallback(GLFWwindow* window, double xpos, double ypos);
    
    const glm::vec3& getCameraFront() const { return currentCameraFront; }
    
private:
    float yaw;
    float pitch;
    float lastX;
    float lastY;
    bool firstMouse;
    float cameraSpeed;
    glm::vec3 currentCameraFront;
    bool cursorEnabled;
    bool graveKeyPressed;
};