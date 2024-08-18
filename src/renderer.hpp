#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <memory>
#include "graphics.hpp"

class Renderer {
   private:
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

    Graphics graphics;

    float angle = 0;

   public:
    Renderer() = default;
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    auto operator=(const Renderer&) -> Renderer& = delete;
    auto operator=(Renderer&&) -> Renderer& = delete;

    auto Initialize(const uint32_t width, const uint32_t height) -> bool;
    void Resize(const uint32_t width, const uint32_t height);
    void Render(const glm::mat4x4 cameraViewMatrix, const glm::mat4x4 projectionMatrix, const float time);

   private:
    auto InitInstance() -> bool;
    auto InitAdapter(const wgpu::Instance& instance) -> bool;
    auto InitDevice(const wgpu::Adapter& adapter) -> bool;
    auto InitSurface(const wgpu::Instance& instance, const wgpu::Adapter& adapter) -> bool;
    auto InitQueue(const wgpu::Device& device) -> bool;
    auto InitDepthBuffer(const wgpu::Device& device, const uint32_t width, const uint32_t height) -> bool;
    auto InitSwapChain(const wgpu::Device& device, const wgpu::Surface& surface, const wgpu::TextureFormat swapChainFormat, const uint32_t width, const uint32_t height) -> bool;
};
