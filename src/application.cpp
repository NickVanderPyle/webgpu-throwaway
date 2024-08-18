#include "application.hpp"
#include <GLFW/glfw3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include "glm/fwd.hpp"
#include "renderer.hpp"

void Application::GetCanvasSize(uint32_t &width, uint32_t &height) {
    EM_ASM(
        {
            var canvas = document.getElementById('canvas');
            var rect = canvas.getBoundingClientRect();
            setValue($0, rect.width, 'i32');
            setValue($1, rect.height, 'i32');
        },
        &width, &height);
}

auto Application::OnPointerLockChangeCallback(int /*eventType*/, const EmscriptenPointerlockChangeEvent *emscEvent, void *userData) -> EM_BOOL {
    auto *app = static_cast<Application *>(userData);

    app->isMousePointerLocked = emscEvent->isActive;

    return EM_TRUE;
}
auto Application::OnMouseMoveCallback(int /*eventType*/, const EmscriptenMouseEvent *mouseEvent, void *userData) -> EM_BOOL {
    auto *app = static_cast<Application *>(userData);

    if (app->isMousePointerLocked) {
        app->mouseDeltaThisFrame.movementX = mouseEvent->movementX;
        app->mouseDeltaThisFrame.movementY = mouseEvent->movementY;
    }

    return EM_TRUE;
}

auto Application::OnMouseButtonCallback(int /*eventType*/, const EmscriptenMouseEvent * /*mouseEvent*/, void *userData) -> EM_BOOL {
    const auto *app = static_cast<const Application *>(userData);

    if (!app->isMousePointerLocked) {
        int result = emscripten_request_pointerlock("#canvas", EM_FALSE);
    }

    return true;
}
auto Application::OnKeyPressCallback(int /*eventType*/, const EmscriptenKeyboardEvent * /*keyEvent*/, void *userData) -> EM_BOOL {
    const auto *app = static_cast<const Application *>(userData);

    emscripten_exit_pointerlock();

    return true;
}

auto Application::InitGlfw() -> bool {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Cannot initialize GLFW" << std::endl;
        return false;
    }
    return true;
}

auto Application::InitializeMouseMovement() -> bool {
    return (emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, this->OnMouseMoveCallback) & ~EMSCRIPTEN_RESULT_DEFERRED) == EMSCRIPTEN_RESULT_SUCCESS
        && (emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, this->OnMouseButtonCallback) & ~EMSCRIPTEN_RESULT_DEFERRED) == EMSCRIPTEN_RESULT_SUCCESS
        && (emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, this->OnMouseButtonCallback) & ~EMSCRIPTEN_RESULT_DEFERRED) == EMSCRIPTEN_RESULT_SUCCESS
        && (emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, this->OnPointerLockChangeCallback) & ~EMSCRIPTEN_RESULT_DEFERRED) == EMSCRIPTEN_RESULT_SUCCESS
        && (emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, this->OnKeyPressCallback) & ~EMSCRIPTEN_RESULT_DEFERRED) == EMSCRIPTEN_RESULT_SUCCESS;
}

auto Application::Initialize() -> bool {
    uint32_t width = 0;
    uint32_t height = 0;
    this->GetCanvasSize(width, height);

    this->camera.Init(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

    return this->InitializeMouseMovement()
        && this->renderer.Initialize(width, height);
}

void Application::Resize(uint32_t width, uint32_t height) {
    this->camera.Resize(width, height);
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

    this->camera.ProcessMouseMovement(this->mouseDeltaThisFrame.movementX, this->mouseDeltaThisFrame.movementY);

    this->renderer.Render(this->camera.GetViewMatrix(), this->camera.GetProjectionMatrix(), time);

    this->mouseDeltaThisFrame.movementX = 0;
    this->mouseDeltaThisFrame.movementY = 0;
}