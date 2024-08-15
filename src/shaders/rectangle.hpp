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
    RectangleShader &operator=(const RectangleShader &) = delete;
    RectangleShader(RectangleShader &&) = delete;
    RectangleShader &operator=(RectangleShader &&) = delete;

    bool Init(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height);
    void Resize(const uint32_t width, const uint32_t height);
    void UpdateBuffers(const std::unique_ptr<wgpu::Queue> &queue, std::vector<glm::mat4x4> &instanceModelMatrices);
    void Render(const std::unique_ptr<wgpu::RenderPassEncoder> &renderPass, const std::unique_ptr<wgpu::Queue> &queue, float time);

   private:
    std::unique_ptr<wgpu::ShaderModule> shaderModule;
    std::unique_ptr<wgpu::RenderPipeline> pipeline;
    std::unique_ptr<wgpu::Buffer> vertexBuffer;
    std::unique_ptr<wgpu::Buffer> instanceBuffer;
    size_t instanceCount;
    size_t maxRectangleCount;
    glm::mat4x4 viewMatrix;
    glm::mat4x4 projectionMatrix;

    bool InitRenderPipeline(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat);
    bool InitVertexBuffer(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue);
    bool InitInstanceBuffer(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue);
};
