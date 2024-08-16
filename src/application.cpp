#include "application.hpp"
#include <GLFW/glfw3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include "renderer.hpp"

auto Application::InitGlfw() -> bool {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Cannot initialize GLFW" << std::endl;
        return false;
    }
    return true;
}

auto Application::Initialize() -> bool {
    return this->renderer.Initialize();
}

void Application::Resize(uint32_t width, uint32_t height) {
    this->renderer.Resize(width, height);
}

void Application::Start() {
    emscripten_set_resize_callback(
        EMSCRIPTEN_EVENT_TARGET_WINDOW,
        this,
        true,
        [](int /*eventType*/, const EmscriptenUiEvent *uiEvent, void *userData) -> EM_BOOL {
            auto *app = static_cast<Application *>(userData);
            app->Resize(uiEvent->windowInnerWidth, uiEvent->windowInnerHeight);
            return EM_TRUE;
        });

    emscripten_set_main_loop_arg(
        [](void *arg) {
            auto *app = static_cast<Application *>(arg);
            app->MainLoop();
        },
        this, 0, (int)true);
}

void Application::MainLoop() {
    glfwPollEvents();
    auto time = static_cast<float>(glfwGetTime());

    this->renderer.Render(time);
}