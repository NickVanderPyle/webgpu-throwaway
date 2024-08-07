#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <memory>
#include "shaderPipeline.hpp"

class Renderer {
   private:
    uint32_t width = 512;
    uint32_t height = 512;

    std::unique_ptr<wgpu::Instance> instance;
    std::unique_ptr<wgpu::Adapter> adapter;
    std::unique_ptr<wgpu::Device> device;
    std::unique_ptr<wgpu::Queue> queue;
    std::unique_ptr<wgpu::Surface> surface;
    wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    std::unique_ptr<wgpu::Texture> depthTexture;
    std::unique_ptr<wgpu::TextureView> depthTextureView;
    std::unique_ptr<wgpu::SwapChain> swapChain;

    ShaderPipeline shaderPipeline;

   public:
    Renderer() = default;
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;
    bool Initialize();
    void Resize(const uint32_t width, const uint32_t height);
    void Render(const float time);

   private:
    void GetCanvasSize(uint32_t& width, uint32_t& height);
    bool InitInstance();
    bool InitAdapter(const std::unique_ptr<wgpu::Instance>& instance);
    bool InitDevice(const std::unique_ptr<wgpu::Adapter>& adapter);
    bool InitSurface(const std::unique_ptr<wgpu::Instance>& instance, const std::unique_ptr<wgpu::Adapter>& adapter);
    bool InitQueue(const std::unique_ptr<wgpu::Device>& device);
    bool InitDepthBuffer(const std::unique_ptr<wgpu::Device>& device, const uint32_t width, const uint32_t height);
    bool InitSwapChain(const std::unique_ptr<wgpu::Device>& device, const std::unique_ptr<wgpu::Surface>& surface, const wgpu::TextureFormat swapChainFormat, const uint32_t width, const uint32_t height);
};
