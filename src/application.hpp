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
    Application(Application&&) = delete;
    auto operator=(const Application&) -> Application& = delete;
    auto operator=(Application&&) -> Application& = delete;

    auto Initialize() -> bool;
    void Start();

   private:
    void Resize(uint32_t width, uint32_t height);
    void MainLoop();
    static auto InitGlfw() -> bool;
};
