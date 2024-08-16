#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <memory>

struct Line3D {
    glm::vec3 start;
    glm::vec3 end;
};

// todo: irritate sonarlint.
class Line3DShader {
   private:
    // Should be the same as in the shader.
    struct MyUniforms {
        glm::mat4x4 modelMatrix;
        glm::mat4x4 viewMatrix;
        glm::mat4x4 projectionMatrix;
        float time;
        float _pad[3];
    };
    // Have the compiler check byte alignment
    static_assert(sizeof(MyUniforms) % 16 == 0);

   public:
    Line3DShader(const size_t maxLineCount);
    ~Line3DShader() = default;
    Line3DShader(const Line3DShader &) = delete;
    Line3DShader(Line3DShader &&) = delete;
    auto operator=(const Line3DShader &) -> Line3DShader & = delete;
    auto operator=(Line3DShader &&) -> Line3DShader & = delete;

    auto Init(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool;
    void Resize(const uint32_t width, const uint32_t height);
    void UpdateVertexBuffer(const wgpu::Queue &queue, const std::vector<Line3D> &lines);
    void Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue &queue, const float time);

   private:
    std::unique_ptr<wgpu::ShaderModule> shaderModule;
    std::unique_ptr<wgpu::BindGroupLayout> bindGroupLayout;
    std::unique_ptr<wgpu::RenderPipeline> pipeline;
    std::unique_ptr<wgpu::Buffer> uniformBuffer;
    std::unique_ptr<wgpu::BindGroup> bindGroup;
    MyUniforms uniforms = MyUniforms();
    std::unique_ptr<wgpu::Buffer> vertexBuffer;
    size_t drawLineCount = 0;
    size_t maxLineCount;

    auto InitBindGroupLayout(const wgpu::Device &device) -> bool;
    auto InitRenderPipeline(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) -> bool;
    auto InitUniforms(const wgpu::Device &device, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool;
    auto InitBindGroup(const wgpu::Device &device, const wgpu::Buffer &uniformBuffer, const wgpu::BindGroupLayout &bindGroupLayout) -> bool;
    auto InitVertexBuffer(const wgpu::Device &device) -> bool;
};
