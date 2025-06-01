#include "render.h"
#include <iostream>
#include <chrono>
#include <clocale>
#include <windows.h>

// 主函数入口点
int main() {
    //设置控制台使用UTF-8编码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "");
    

    // 创建渲染器实例
    Renderer renderer;
    
    // 初始化渲染器，设置窗口大小为1920, 1080,
    if (!renderer.init(1920, 1080, "Renderer")) {
        // 如果初始化失败，输出错误信息并退出
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }

    // 记录程序开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    // 主渲染循环
    while (!renderer.shouldClose()) {
        // 获取当前时间
        auto currentTime = std::chrono::high_resolution_clock::now();
        // 计算从程序开始到现在经过的时间（以秒为单位）
        float time = std::chrono::duration<float>(currentTime - startTime).count();
        
        // 执行渲染，传入当前时间作为参数
        renderer.render(time);
    }

    // 程序正常结束
    return 0;
}