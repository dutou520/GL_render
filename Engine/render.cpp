#include "render.h"
#define GLAD_GL_IMPLEMENTATION
#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include "shader.h"
#include <spdlog/spdlog.h>
// �����λ��
#define CAMERA_POS glm::vec3(0.0f, 2.0f, 8.0f)
// ����MSAA
#define ENABLE_MSAA 1
// ����MSAA������
#define MSAA_SAMPLES 4

Renderer* Renderer::currentInstance = nullptr;
// ���캯��
Renderer::Renderer() : window(nullptr), shaderProgram(0), VAO(0), VBO(0), EBO(0),
    cameraPos(CAMERA_POS),
    cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
    cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    lastFrameTime(0.0),
    targetFrameTime(1.0 / 60.0),
    windowWidth(1920),
    windowHeight(1080),
    PBR_shader(){
    currentInstance = this;
}
// ��������
Renderer::~Renderer() {
    cleanup();
}
// ��ʼ����Ⱦ��
bool Renderer::init(int width, int height, const char* title) {
    // ��ʼ��GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    // ����GLFW����ص�
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    });
    
    // ����OpenGL�汾Ϊ3.3��ʹ�ú���ģʽ
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if ENABLE_MSAA
    // ����MSAA������
    glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES);
#endif

    // ��������
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // ���õ�ǰ������
    glfwMakeContextCurrent(window);

    // ���ô�ֱͬ��
    glfwSwapInterval(1);

    // ��ʼ�����������
    inputManager.init(window);

    // ��ʼ��GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

#if ENABLE_MSAA
    // ���ö��ز���
    glEnable(GL_MULTISAMPLE);
#endif

    // ���ô��ڳߴ�
    windowWidth = width;
    windowHeight = height;
    
    // �����ӿ�
    glViewport(0, 0, width, height);
    
    // ���ô��ڴ�С�仯�ص�
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    
    // ������Ȳ���
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // ��ʼ��ImGui
    guiRenderer.initImGui(window);
    
    // ����GUI��Ⱦ��
    guiRenderer.init();

    setupPBRShader();

    // ���ز��Է���ģ��
    loadTestRoom();

    return true;
}
// ��ɫ������
void Renderer::setupPBRShader() {
    const char* vertexPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Shader/pbrshader.vert";
    const char* fragmentPath = "c:/Users/dutou/Documents/CppPrograms/GL_Render/Shader/pbrshader.frag";
    // ���Լ��غͱ�����ɫ���ļ�
    try {
        PBR_shader = Shader(vertexPath, fragmentPath);
        shaderProgram = PBR_shader.ID;
        // ����ɹ���Ϣ����־
        spdlog::info("Shader loaded successfully at ID {}", shaderProgram);
    } 
    // ������ɫ�����ع����е��쳣
    catch(const std::exception& e) {
        // ���������Ϣ����׼������
        spdlog::error("Shader loading failed: {}", e.what());
        return;
    }
}
// ֡�ʿ���
void Renderer::controlFrameRate() {
    static double frameTimeSum = 0.0;
    static int frameCount = 0;
    static double lastFPSUpdateTime = glfwGetTime();
    
    // ��ȡ��ǰ֡��ʼʱ��
    double frameStartTime = glfwGetTime();
    double deltaTime = frameStartTime - lastFrameTime;
    
    // ���㲢��ʾFPS
    frameTimeSum += deltaTime;
    frameCount++;
    if (frameStartTime - lastFPSUpdateTime >= 1.0) {
        double averageFPS = frameCount / (frameStartTime - lastFPSUpdateTime);
        // ���FPS����־
        //����һ��
        std::cout << "\033[1A";
        spdlog::info("FPS: {:.2f}", averageFPS);
        frameTimeSum = 0.0;
        frameCount = 0;
        lastFPSUpdateTime = frameStartTime;
    }
    
    // ���������һ֡ʱ��С��Ŀ��֡ʱ�䣬���о�ȷ�ȴ�
    if (deltaTime < targetFrameTime) {
        double waitTime = targetFrameTime - deltaTime;
        double waitStartTime = glfwGetTime();
        while (glfwGetTime() - waitStartTime < waitTime) {
            std::this_thread::yield(); // �ó�CPUʱ��Ƭ������CPUռ��
        }
    }
    
    // ������һ֡ʱ��
    lastFrameTime = frameStartTime;
}

void Renderer::render(float time) {
    // ֡�ʿ���
    controlFrameRate();

    // �����ɫ����Ȼ���
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // ȷ����Ȳ��Ժ�����ƽ��������
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
    
    // �����ɫ�������Ƿ���Ч
    if (!shaderProgram) {
        spdlog::error("Error: Shader program is not valid");
        return;
    }
    
    // �������벢�����������
    inputManager.processInput(window, cameraPos, cameraFront, cameraUp);
    cameraFront = inputManager.getCameraFront();
    
    // ���ñ任����
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp
    );
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 100.0f);
    
    // ����GUI��Ⱦ���ı任����
    guiRenderer.setViewMatrix(view);
    guiRenderer.setProjectionMatrix(projection);
    
    // ���Ʋο�ϵͳ
    guiRenderer.renderGrid();
    guiRenderer.renderAxis();

    // ��Ⱦ����
    scene.render(PBR_shader, view, projection);
    
    // ��ȾImGui����
    guiRenderer.renderImGui(scene);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::cleanup() {
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Renderer::loadTestRoom() {
    if (!scene.loadModel("test_room.obj")) {
        spdlog::error("�޷�����TestRoomģ��");
    }
}

void Renderer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (currentInstance) {
        currentInstance->windowWidth = width;
        currentInstance->windowHeight = height;
        glViewport(0, 0, width, height);
    }
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(window);
}