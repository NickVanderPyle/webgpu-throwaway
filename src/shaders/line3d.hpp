#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <memory>

struct Line3D {
    glm::vec3 start;
    glm::vec3 end;
};

class Line3DShader {
   private:
    struct VertexAttributes {
        glm::vec3 position;
        glm::vec3 color;
    };

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
    Line3DShader(size_t maxLineCount);
    ~Line3DShader() = default;
    Line3DShader(const Line3DShader &) = delete;
    Line3DShader &operator=(const Line3DShader &) = delete;
    Line3DShader(Line3DShader &&) = delete;
    Line3DShader &operator=(Line3DShader &&) = delete;

    bool Init(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height);
    void Resize(const uint32_t width, const uint32_t height);
    void UpdateVertexBuffer(const std::unique_ptr<wgpu::Queue> &queue, std::vector<Line3D> &lines);
    void Render(const std::unique_ptr<wgpu::RenderPassEncoder> &renderPass, const std::unique_ptr<wgpu::Queue> &queue, float time);

   private:
    std::unique_ptr<wgpu::ShaderModule> shaderModule;
    std::unique_ptr<wgpu::BindGroupLayout> bindGroupLayout;
    std::unique_ptr<wgpu::RenderPipeline> pipeline;
    std::unique_ptr<wgpu::Buffer> uniformBuffer;
    std::unique_ptr<wgpu::BindGroup> bindGroup;
    MyUniforms uniforms;
    std::unique_ptr<wgpu::Buffer> vertexBuffer;
    size_t drawLineCount = 0;
    size_t maxLineCount;

    bool InitBindGroupLayout(const std::unique_ptr<wgpu::Device> &device);
    bool InitRenderPipeline(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat);
    bool InitUniforms(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height);
    bool InitBindGroup(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Buffer> &uniformBuffer, const std::unique_ptr<wgpu::BindGroupLayout> &bindGroupLayout);
    bool InitVertexBuffer(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue);
};
