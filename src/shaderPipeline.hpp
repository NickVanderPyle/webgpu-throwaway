#pragma once

#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <memory>
#include "model.hpp"

class ShaderPipeline {
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
    ShaderPipeline() = default;
    ~ShaderPipeline() = default;
    ShaderPipeline(const ShaderPipeline &) = delete;
    ShaderPipeline &operator=(const ShaderPipeline &) = delete;
    ShaderPipeline(ShaderPipeline &&) = delete;
    ShaderPipeline &operator=(ShaderPipeline &&) = delete;

    bool Init(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height);
    void Resize(const uint32_t width, const uint32_t height);
    void Render(const std::unique_ptr<wgpu::RenderPassEncoder> &renderPass, const std::unique_ptr<wgpu::Queue> &queue, float time);

   private:
    std::unique_ptr<wgpu::ShaderModule> shaderModule;
    std::unique_ptr<wgpu::BindGroupLayout> bindGroupLayout;
    std::unique_ptr<wgpu::RenderPipeline> pipeline;
    std::unique_ptr<wgpu::Buffer> uniformBuffer;
    std::unique_ptr<wgpu::BindGroup> bindGroup;
    MyUniforms uniforms;
    std::unique_ptr<ModelData> model;

    bool InitBindGroupLayout(const std::unique_ptr<wgpu::Device> &device);
    bool InitRenderPipeline(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat);
    bool InitUniforms(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height);
    bool InitBindGroup(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Buffer> &uniformBuffer, const std::unique_ptr<wgpu::BindGroupLayout> &bindGroupLayout);
};
