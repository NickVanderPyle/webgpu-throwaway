#include "graphics.hpp"
#include "camera.hpp"

Graphics::Graphics()
    : line3d_shader(std::make_unique<Line3DShader>(Graphics::line3d_maxLineCount)),
      cube_shader(std::make_unique<CubeShader>(Graphics::cube_maxCubeCount)) {
    this->line3d_lines.reserve(Graphics::line3d_maxLineCount);
    this->cube_instanceModelMatrices.reserve(Graphics::cube_maxCubeCount);
}

auto Graphics::InitShaders(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool {
    return this->line3d_shader->Init(device, swapChainFormat, depthTextureFormat, queue, width, height)
        && this->cube_shader->Init(device, swapChainFormat, depthTextureFormat, queue);
}

void Graphics::DrawLine(const glm::vec3 start, const glm::vec3 end, const glm::vec3 /*color*/) {
    if (this->line3d_lines.size() >= Graphics::line3d_maxLineCount) {
        return;
    }

    this->line3d_lines.push_back(Line3D{start, end});
}

void Graphics::DrawRect(const glm::mat4x4 transform) {
    if (this->cube_instanceModelMatrices.size() >= Graphics::cube_maxCubeCount) {
        return;
    }

    this->cube_instanceModelMatrices.push_back(transform);
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

    if (!this->cube_instanceModelMatrices.empty()) {
        this->cube_shader->UpdateBuffers(queue, this->cube_instanceModelMatrices);
        this->cube_shader->Render(renderPass, queue, cameraViewMatrix, projectionMatrix, time);
        this->cube_instanceModelMatrices.clear();
    }
}