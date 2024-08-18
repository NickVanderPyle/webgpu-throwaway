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

// Should be the same as in the shader.
struct MyUniforms {
    glm::mat4x4 viewMatrix;
    glm::mat4x4 projectionMatrix;
    float time;
    float _pad[3];
};
// Have the compiler check byte alignment
static_assert(sizeof(MyUniforms) % 16 == 0);

class RectangleShader {
   public:
    RectangleShader(size_t maxRectangleCount);
    ~RectangleShader() = default;
    RectangleShader(const RectangleShader &) = delete;
    RectangleShader(RectangleShader &&) = delete;
    auto operator=(const RectangleShader &) -> RectangleShader & = delete;
    auto operator=(RectangleShader &&) -> RectangleShader & = delete;

    auto Init(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue) -> bool;
    void UpdateBuffers(const wgpu::Queue &queue, std::vector<glm::mat4x4> &instanceModelMatrices);
    void Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue &queue, const glm::mat4x4 cameraViewMatrix, const glm::mat4x4 projectionMatrix, const float time);

   private:
    std::unique_ptr<wgpu::ShaderModule> shaderModule;
    std::unique_ptr<wgpu::BindGroupLayout> bindGroupLayout;
    std::unique_ptr<wgpu::RenderPipeline> pipeline;
    std::unique_ptr<wgpu::Buffer> uniformBuffer;
    std::unique_ptr<wgpu::BindGroup> bindGroup;
    std::unique_ptr<wgpu::Buffer> vertexBuffer;
    std::unique_ptr<wgpu::Buffer> instanceBuffer;
    MyUniforms uniforms = MyUniforms();
    size_t instanceCount = 0;
    size_t maxRectangleCount;

    auto InitBindGroupLayout(const wgpu::Device &device) -> bool;
    auto InitRenderPipeline(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) -> bool;
    auto InitVertexBuffer(const wgpu::Device &device, const wgpu::Queue &queue) -> bool;
    auto InitUniforms(const wgpu::Device &device) -> bool;
    auto InitBindGroup(const wgpu::Device &device, const wgpu::Buffer &uniformBuffer, const wgpu::BindGroupLayout &bindGroupLayout) -> bool;
    auto InitInstanceBuffer(const wgpu::Device &device) -> bool;
};
