#pragma once
#include <webgpu/webgpu_cpp.h>
#include "renderer.hpp"

class Application {
   private:
    Renderer renderer;

   public:
    Application() = default;
    ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    bool Initialize();
    void Start();

   private:
    void Resize(uint32_t width, uint32_t height);
    void MainLoop();
    bool InitGlfw();
};
