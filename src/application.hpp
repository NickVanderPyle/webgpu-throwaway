#pragma once
#include <emscripten/html5.h>
#include <webgpu/webgpu_cpp.h>
#include "camera.hpp"
#include "renderer.hpp"

struct MouseDelta {
    int movementX;
    int movementY;
};

struct Point {
    int x;
    int y;
};

class Application {
   private:
    Renderer renderer;
    Camera camera;

   public:
    Application() = default;
    ~Application() = default;
    Application(const Application &) = delete;
    Application(Application &&) = delete;
    auto operator=(const Application &) -> Application & = delete;
    auto operator=(Application &&) -> Application & = delete;

    auto Initialize() -> bool;
    void Start();

   private:
    bool isMousePointerLocked = false;
    MouseDelta mouseDeltaThisFrame = MouseDelta{.movementX = 0, .movementY = 0};
    Point lastTouchPoint = Point();

    static void GetCanvasSize(uint32_t &width, uint32_t &height);
    static auto OnTouchMoveCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) -> EM_BOOL;
    static auto OnPointerLockChangeCallback(int /*eventType*/, const EmscriptenPointerlockChangeEvent *emscEvent, void *userData) -> EM_BOOL;
    static auto OnMouseMoveCallback(int /*eventType*/, const EmscriptenMouseEvent * /*mouseEvent*/, void *userData) -> EM_BOOL;
    static auto OnMouseButtonCallback(int /*eventType*/, const EmscriptenMouseEvent * /*mouseEvent*/, void *userData) -> EM_BOOL;
    static auto OnKeyPressCallback(int /*eventType*/, const EmscriptenKeyboardEvent * /*keyEvent*/, void *userData) -> EM_BOOL;
    auto InitializeMouseMovement() -> bool;
    void Resize(uint32_t width, uint32_t height);
    void MainLoop();
    static auto InitGlfw() -> bool;
};
