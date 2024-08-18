#include "camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

void Camera::SetPosition(const glm::vec3 position) {
    this->position = position;
}

void Camera::Init(uint32_t width, uint32_t height, glm::vec3 position) {
    this->Resize(width, height);
    this->SetPosition(position);
}

void Camera::Resize(uint32_t width, uint32_t height) {
    this->projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
}

void Camera::ProcessMouseMovement(const int mouseDeltaX, const int mouseDeltaY) {
    float deltaX = (float)mouseDeltaX * sensitivity;
    float deltaY = (float)mouseDeltaY * sensitivity;

    // Update yaw and pitch
    this->yaw += deltaX;
    pitch -= deltaY;  // Inverted because y-coordinates go from bottom to top

    // Constrain the pitch
    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    // Calculate the new front vector
    glm::vec3 front{
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
    };
    cameraFront = glm::normalize(front);
}

auto Camera::GetViewMatrix() -> glm::mat4x4 {
    // Create the view matrix using glm::lookAt
    return glm::lookAt(this->position, this->position + cameraFront, cameraUp);
}

auto Camera::GetProjectionMatrix() -> glm::mat4x4 {
    return this->projectionMatrix;
}