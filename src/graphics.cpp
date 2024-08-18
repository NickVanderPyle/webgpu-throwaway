#include "graphics.hpp"
#include "camera.hpp"

Graphics::Graphics()
    : line3d_shader(std::make_unique<Line3DShader>(Graphics::line3d_maxLineCount)),
      rectangle_shader(std::make_unique<RectangleShader>(Graphics::rectangle_maxRectangleCount)) {
    this->line3d_lines.reserve(Graphics::line3d_maxLineCount);
    this->rectangle_instanceModelMatrices.reserve(Graphics::rectangle_maxRectangleCount);
}

auto Graphics::InitShaders(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool {
    return this->line3d_shader->Init(device, swapChainFormat, depthTextureFormat, queue, width, height)
        && this->rectangle_shader->Init(device, swapChainFormat, depthTextureFormat, queue);
}

void Graphics::DrawLine(const glm::vec3 start, const glm::vec3 end, const glm::vec3 /*color*/) {
    if (this->line3d_lines.size() >= Graphics::line3d_maxLineCount) {
        return;
    }

    this->line3d_lines.push_back(Line3D{start, end});
}

void Graphics::DrawRect(const glm::mat4x4 transform) {
    if (this->rectangle_instanceModelMatrices.size() >= Graphics::rectangle_maxRectangleCount) {
        return;
    }

    this->rectangle_instanceModelMatrices.push_back(transform);
}

void Graphics::Resize(const uint32_t width, const uint32_t height) {
    this->line3d_shader->Resize(width, height);
}

void Graphics::Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue &queue, const glm::mat4x4 cameraViewMatrix, const glm::mat4x4 projectionMatrix, const float time) {
    if (!this->line3d_lines.empty()) {
        this->line3d_shader->UpdateVertexBuffer(queue, this->line3d_lines);
        this->line3d_shader->Render(renderPass, queue, time);
        this->line3d_lines.clear();
    }

    if (!this->rectangle_instanceModelMatrices.empty()) {
        this->rectangle_shader->UpdateBuffers(queue, this->rectangle_instanceModelMatrices);
        this->rectangle_shader->Render(renderPass, queue, cameraViewMatrix, projectionMatrix, time);
        this->rectangle_instanceModelMatrices.clear();
    }
}