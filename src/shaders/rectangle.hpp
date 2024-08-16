#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Rectangle {
    glm::vec3 topLeft;
    glm::vec3 bottomLeft;
    glm::vec3 topRight;
    glm::vec3 bottomRight;
};

class RectangleShader {
   public:
    RectangleShader(size_t maxRectangleCount);
    ~RectangleShader() = default;
    RectangleShader(const RectangleShader &) = delete;
    RectangleShader(RectangleShader &&) = delete;
    auto operator=(const RectangleShader &) -> RectangleShader & = delete;
    auto operator=(RectangleShader &&) -> RectangleShader & = delete;

    auto Init(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool;
    void Resize(const uint32_t width, const uint32_t height);
    void UpdateBuffers(const wgpu::Queue &queue, std::vector<glm::mat4x4> &instanceModelMatrices);
    void Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue &queue, const float time);

   private:
    std::unique_ptr<wgpu::ShaderModule> shaderModule;
    std::unique_ptr<wgpu::RenderPipeline> pipeline;
    std::unique_ptr<wgpu::Buffer> vertexBuffer;
    std::unique_ptr<wgpu::Buffer> instanceBuffer;
    size_t instanceCount = 0;
    size_t maxRectangleCount;
    glm::mat4x4 viewMatrix = glm::mat4x4();
    glm::mat4x4 projectionMatrix = glm::mat4x4();

    auto InitRenderPipeline(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) -> bool;
    auto InitVertexBuffer(const wgpu::Device &device, const wgpu::Queue &queue) -> bool;
    auto InitInstanceBuffer(const wgpu::Device &device) -> bool;
};
