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
    EM_ASM(
        {
            var canvas = document.getElementById('canvas');
            var rect = canvas.getBoundingClientRect();
            setValue($0, rect.width, 'i32');
            setValue($1, rect.height, 'i32');
        },
        &width, &height);
}

auto Renderer::InitInstance() -> bool {
    this->instance = std::make_unique<wgpu::Instance>(wgpu::CreateInstance());
    if (!this->instance) {
        std::cerr << "Cannot initialize WebGPU Instance" << std::endl;
        return false;
    }
    return true;
}

auto Renderer::InitAdapter(const wgpu::Instance &instance) -> bool {
    struct UserData {
        std::unique_ptr<wgpu::Adapter> adapter;
        bool requestEnded = false;
    } userData;

    instance.RequestAdapter(
        nullptr,
        [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *pUserData) {
            UserData &userData = *static_cast<UserData *>(pUserData);

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

auto Renderer::InitDevice(const wgpu::Adapter &adapter) -> bool {
    struct UserData {
        std::unique_ptr<wgpu::Device> device;
        bool requestEnded = false;
    } userData;

    adapter.RequestDevice(
        nullptr,
        [](WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *pUserData) {
            UserData &userData = *static_cast<UserData *>(pUserData);

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

auto Renderer::InitQueue(const wgpu::Device &device) -> bool {
    this->queue = std::make_unique<wgpu::Queue>(device.GetQueue());
    if (!this->queue) {
        std::cerr << "Cannot initialize WebGPU Queue" << std::endl;
        return false;
    }
    return true;
}

auto Renderer::InitDepthBuffer(const wgpu::Device &device) -> bool {
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
    this->depthTexture = std::make_unique<wgpu::Texture>(device.CreateTexture(&depthTextureDesc));
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

auto Renderer::InitSurface(const wgpu::Instance &instance, const wgpu::Adapter &adapter) -> bool {
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc;
    canvasDesc.selector = "#canvas";
    wgpu::SurfaceDescriptor surfaceDesc{
        .nextInChain = &canvasDesc,
        .label = "Renderer",
    };
    this->surface = std::make_unique<wgpu::Surface>(instance.CreateSurface(&surfaceDesc));

    if (!this->surface) {
        std::cerr << "Cannot initialize WebGPU Surface" << std::endl;
        return false;
    }

    this->swapChainFormat = this->surface->GetPreferredFormat(adapter.Get());

    return true;
}

auto Renderer::InitSwapChain(const wgpu::Device &device, const wgpu::Surface &surface, const wgpu::TextureFormat swapChainFormat, const uint32_t width, const uint32_t height) -> bool {
    wgpu::SwapChainDescriptor swapChainDesc{
        .label = "Renderer",
        .usage = wgpu::TextureUsage::RenderAttachment,
        .format = swapChainFormat,
        .width = width,
        .height = height,
        .presentMode = wgpu::PresentMode::Fifo,
    };

    this->swapChain = std::make_unique<wgpu::SwapChain>(device.CreateSwapChain(surface.Get(), &swapChainDesc));

    if (!this->swapChain) {
        std::cerr << "Cannot initialize WebGPU SwapChain" << std::endl;
        return false;
    }

    return true;
}

auto Renderer::Initialize() -> bool {
    this->GetCanvasSize(this->width, this->height);

    return this->InitInstance()
        && this->InitAdapter(this->instance->Get())
        && this->InitDevice(this->adapter->Get())
        && this->InitSurface(this->instance->Get(), this->adapter->Get())
        && this->InitSwapChain(this->device->Get(), this->surface->Get(), this->swapChainFormat, this->width, this->height)
        && this->InitQueue(this->device->Get())
        && this->InitDepthBuffer(this->device->Get())
        && this->graphics.InitShaders(this->device->Get(), this->swapChainFormat, this->depthTextureFormat, this->queue->Get(), this->width, this->height);
}

void Renderer::Resize(const uint32_t width, const uint32_t height) {
    this->width = width;
    this->height = height;
    this->InitSwapChain(this->device->Get(), this->surface->Get(), this->swapChainFormat, width, height);
    this->InitDepthBuffer(this->device->Get());
    this->graphics.Resize(width, height);
}

void Renderer::Render(const float time) {
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

        auto renderPass = encoder.BeginRenderPass(&renderPassDesc);

        this->angle++;
        float angleTmp = 0;
        for (int x = -120; x < 120; x += 10) {
            for (int y = -100; y < 100; y += 10) {
                auto transform = glm::mat4x4(1.0f);
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

        this->graphics.Render(renderPass, this->queue->Get(), time);

        renderPass.End();
    }

    wgpu::CommandBuffer command = encoder.Finish();
    this->queue->Submit(1, &command);
}