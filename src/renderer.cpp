#include "renderer.hpp"
#include <emscripten/emscripten.h>
#include <webgpu/webgpu_cpp.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>

void Renderer::GetCanvasSize(uint32_t &width, uint32_t &height) {
    EM_ASM({
        var canvas = document.getElementById('canvas');
        var rect = canvas.getBoundingClientRect();
        setValue($0, rect.width, 'i32');
        setValue($1, rect.height, 'i32'); }, &width, &height);
}

bool Renderer::InitInstance() {
    this->instance = std::make_unique<wgpu::Instance>(wgpu::CreateInstance());
    if (!this->instance) {
        std::cerr << "Cannot initialize WebGPU Instance" << std::endl;
        return false;
    }
    return true;
}

bool Renderer::InitAdapter(const std::unique_ptr<wgpu::Instance> &instance) {
    struct UserData {
        std::unique_ptr<wgpu::Adapter> adapter;
        bool requestEnded = false;
    } userData;

    instance->RequestAdapter(
        nullptr,
        [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *pUserData) {
            UserData &userData = *reinterpret_cast<UserData *>(pUserData);

            if (status == WGPURequestAdapterStatus::WGPURequestAdapterStatus_Success) {
                userData.adapter = std::make_unique<wgpu::Adapter>(wgpu::Adapter::Acquire(cAdapter));
                if (!userData.adapter) {
                    std::cerr << "Could not acquire adapter" << std::endl;
                }
            } else {
                std::cerr << "Could not get WebGPU adapter: " << message << std::endl;
            }

            userData.requestEnded = true;
        },
        &userData);

    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }

    this->adapter = std::move(userData.adapter);

    return static_cast<bool>(this->adapter);
}

bool Renderer::InitDevice(const std::unique_ptr<wgpu::Adapter> &adapter) {
    struct UserData {
        std::unique_ptr<wgpu::Device> device;
        bool requestEnded = false;
    } userData;

    adapter->RequestDevice(
        nullptr,
        [](WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *pUserData) {
            UserData &userData = *reinterpret_cast<UserData *>(pUserData);

            if (status == WGPURequestDeviceStatus::WGPURequestDeviceStatus_Success) {
                userData.device = std::make_unique<wgpu::Device>(wgpu::Device::Acquire(cDevice));
                if (!userData.device) {
                    std::cerr << "Could not acquire device" << std::endl;
                }
            } else {
                std::cerr << "Could not get WebGPU device: " << message << std::endl;
            }

            userData.requestEnded = true;
        },
        &userData);

    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }

    this->device = std::move(userData.device);

    return static_cast<bool>(this->device);
}

bool Renderer::InitQueue(const std::unique_ptr<wgpu::Device> &device) {
    this->queue = std::make_unique<wgpu::Queue>(device->GetQueue());
    if (!this->queue) {
        std::cerr << "Cannot initialize WebGPU Queue" << std::endl;
        return false;
    }
    return true;
}

bool Renderer::InitDepthBuffer(const std::unique_ptr<wgpu::Device> &device, const uint32_t width, const uint32_t height) {
    wgpu::TextureDescriptor depthTextureDesc{
        .label = "Renderer",
        .usage = wgpu::TextureUsage::RenderAttachment,
        .dimension = wgpu::TextureDimension::e2D,
        .size = {this->width, this->height, 1},
        .format = this->depthTextureFormat,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 1,
        .viewFormats = &this->depthTextureFormat,
    };
    this->depthTexture = std::make_unique<wgpu::Texture>(device->CreateTexture(&depthTextureDesc));
    if (!this->depthTexture) {
        std::cerr << "Cannot initialize WebGPU DepthTexture" << std::endl;
        return false;
    }

    wgpu::TextureViewDescriptor depthTextureViewDesc{
        .label = "Renderer",
        .format = this->depthTextureFormat,
        .dimension = wgpu::TextureViewDimension::e2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::DepthOnly,
    };
    this->depthTextureView = std::make_unique<wgpu::TextureView>(this->depthTexture->CreateView(&depthTextureViewDesc));
    if (!this->depthTextureView) {
        std::cerr << "Cannot initialize WebGPU DepthTextureView" << std::endl;
        return false;
    }

    return true;
}

bool Renderer::InitSurface(const std::unique_ptr<wgpu::Instance> &instance, const std::unique_ptr<wgpu::Adapter> &adapter) {
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc;
    canvasDesc.selector = "#canvas";
    wgpu::SurfaceDescriptor surfaceDesc{
        .nextInChain = &canvasDesc,
        .label = "Renderer",
    };
    this->surface = std::make_unique<wgpu::Surface>(instance->CreateSurface(&surfaceDesc));

    if (!this->surface) {
        std::cerr << "Cannot initialize WebGPU Surface" << std::endl;
        return false;
    }

    this->swapChainFormat = this->surface->GetPreferredFormat(adapter->Get());

    return true;
}

bool Renderer::InitSwapChain(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Surface> &surface, const wgpu::TextureFormat swapChainFormat, const uint32_t width, const uint32_t height) {
    wgpu::SwapChainDescriptor swapChainDesc{
        .label = "Renderer",
        .usage = wgpu::TextureUsage::RenderAttachment,
        .format = swapChainFormat,
        .width = width,
        .height = height,
        .presentMode = wgpu::PresentMode::Fifo,
    };

    this->swapChain = std::make_unique<wgpu::SwapChain>(device->CreateSwapChain(surface->Get(), &swapChainDesc));

    if (!this->swapChain) {
        std::cerr << "Cannot initialize WebGPU SwapChain" << std::endl;
        return false;
    }

    return true;
}

bool Renderer::Initialize() {
    this->GetCanvasSize(this->width, this->height);
    if (!this->InitInstance()) return false;
    if (!this->InitAdapter(this->instance)) return false;
    if (!this->InitDevice(this->adapter)) return false;
    if (!this->InitSurface(this->instance, this->adapter)) return false;
    if (!this->InitSwapChain(this->device, this->surface, this->swapChainFormat, this->width, this->height)) return false;
    if (!this->InitQueue(this->device)) return false;
    if (!this->InitDepthBuffer(this->device, this->width, this->height)) return false;

    std::cout << "swapChainFormat:" << static_cast<uint32_t>(this->swapChainFormat) << " depthTextureFormat:" << static_cast<uint32_t>(this->depthTextureFormat) << std::endl;
    if (!this->graphics.InitShaders(this->device, this->swapChainFormat, this->depthTextureFormat, this->queue, this->width, this->height)) return false;

    return true;
}

void Renderer::Resize(uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;
    this->InitSwapChain(this->device, this->surface, this->swapChainFormat, width, height);
    this->InitDepthBuffer(this->device, width, height);
    this->graphics.Resize(width, height);
}

void Renderer::Render(float time) {
    wgpu::TextureView nextTexture = this->swapChain->GetCurrentTextureView();
    if (!nextTexture) {
        std::cerr << "Failed to get nextTexture." << std::endl;
        return;
    }

    wgpu::CommandEncoder encoder = this->device->CreateCommandEncoder();

    {  // Render pass

        wgpu::RenderPassColorAttachment renderPassColorAttachment{
            .view = nextTexture,
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = wgpu::Color{0.05, 0.05, 0.05, 1.0},
        };
        wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment{
            .view = this->depthTextureView->Get(),
            .depthLoadOp = wgpu::LoadOp::Clear,
            .depthStoreOp = wgpu::StoreOp::Store,
            .depthClearValue = 1.0f,
            .depthReadOnly = false,
            // Stencil is not used
            .stencilLoadOp = wgpu::LoadOp::Undefined,
            .stencilStoreOp = wgpu::StoreOp::Undefined,
            .stencilClearValue = 0,
            .stencilReadOnly = true,
        };
        wgpu::RenderPassDescriptor renderPassDesc{
            .label = "Renderer",
            .colorAttachmentCount = 1,
            .colorAttachments = &renderPassColorAttachment,
            .depthStencilAttachment = &renderPassDepthStencilAttachment,
            .timestampWrites = nullptr,
        };

        auto renderPass = std::make_unique<wgpu::RenderPassEncoder>(encoder.BeginRenderPass(&renderPassDesc));

        this->angle++;
        float angleTmp = 0;
        for (int x = -120; x < 120; x += 10) {
            for (int y = -100; y < 100; y += 10) {
                glm::mat4x4 transform = glm::mat4x4(1.0f);
                transform = glm::translate(transform, glm::vec3(x, y, 0));                                       // position
                transform = glm::rotate(transform, glm::radians(this->angle + angleTmp++), glm::vec3(1, 0, 0));  // rotation x
                angleTmp++;
                transform = glm::rotate(transform, glm::radians(this->angle + angleTmp++), glm::vec3(0, 1, 0));  // rotation y
                angleTmp++;
                transform = glm::rotate(transform, glm::radians(this->angle + angleTmp++), glm::vec3(0, 0, 1));  // rotation z
                angleTmp++;
                transform = glm::scale(transform, glm::vec3(5.0f, 5.0f, 5.0f));  // scale

                this->graphics.DrawRect(transform);
            }
        }

        this->graphics.Render(renderPass, this->queue, time);

        renderPass->End();
    }

    wgpu::CommandBuffer command = encoder.Finish();
    this->queue->Submit(1, &command);
}