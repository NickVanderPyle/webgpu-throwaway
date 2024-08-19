#pragma once
#include <glm/glm.hpp>
#include "glm/fwd.hpp"

class Camera {
   public:
    Camera() = default;
    ~Camera() = default;
    Camera(const Camera &) = delete;
    Camera(Camera &&) = delete;
    auto operator=(const Camera &) -> Camera & = delete;
    auto operator=(Camera &&) -> Camera & = delete;

    void Init(uint32_t width, uint32_t height, glm::vec3 position);
    void Resize(uint32_t width, uint32_t height);
    void SetPosition(const glm::vec3 position);
    void ProcessMouseMovement(const int mouseDeltaX, const int mouseDeltaY);
    auto GetViewMatrix() -> glm::mat4x4;
    auto GetProjectionMatrix() -> glm::mat4x4;

   private:
    float yaw = -90.0f;  // Yaw is initialized to -90 degrees since a forward vector points in the negative Z direction by default.
    float pitch = 0.0f;

    float sensitivity = 0.3f;

    glm::mat4x4 projectionMatrix = glm::mat4x4(1);
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
};
