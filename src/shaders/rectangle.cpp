#include "rectangle.hpp"
#include <cstddef>
#include "../resourceManager.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

// offsetof from std lib has trouble with VSCode intellisense, override.
template <typename T, typename U>
constexpr size_t offsetOfMember(U T::*member) {
    return (char *)&((T *)nullptr->*member) - (char *)nullptr;
}

RectangleShader::RectangleShader(size_t maxRectangleCount) : maxRectangleCount(maxRectangleCount) {}

bool RectangleShader::InitRenderPipeline(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) {
    this->shaderModule = ResourceManager::LoadShaderModule("/src/shaders/rectangle.wgsl", device);

    // VertexAttributes::positiom
    std::array<wgpu::VertexAttribute, 1> vertexAttribs{
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = 0,
            .shaderLocation = 0,
        },
    };
    wgpu::VertexBufferLayout vertexBufferLayout{
        .arrayStride = sizeof(glm::vec3),
        .stepMode = wgpu::VertexStepMode::Vertex,
        .attributeCount = (uint32_t)vertexAttribs.size(),
        .attributes = vertexAttribs.data(),
    };

    // VertexAttributes::modelMatrix
    std::array<wgpu::VertexAttribute, 4> instanceAttribs;
    for (uint32_t i = 0; i < instanceAttribs.size(); i++) {
        instanceAttribs[i] = wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x4,
            .offset = sizeof(glm::vec4) * i,  // glm::mat4x4, each row is vec4
            .shaderLocation = 1 + i,
        };
    }
    wgpu::VertexBufferLayout instanceBufferLayout{
        .arrayStride = sizeof(glm::mat4x4),
        .stepMode = wgpu::VertexStepMode::Instance,
        .attributeCount = (uint32_t)instanceAttribs.size(),
        .attributes = instanceAttribs.data(),
    };

    wgpu::BlendState blendState{
        .color = wgpu::BlendComponent{
            .operation = wgpu::BlendOperation::Add,
            .srcFactor = wgpu::BlendFactor::SrcAlpha,
            .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
        },
        .alpha = wgpu::BlendComponent{
            .operation = wgpu::BlendOperation::Add,
            .srcFactor = wgpu::BlendFactor::Zero,
            .dstFactor = wgpu::BlendFactor::One,
        }};

    wgpu::ColorTargetState colorTarget{
        .format = swapChainFormat,
        .blend = &blendState,
        .writeMask = wgpu::ColorWriteMask::All,
    };

    wgpu::FragmentState fragmentState{
        .module = this->shaderModule->Get(),
        .entryPoint = "fs_main",
        .constantCount = 0,
        .constants = nullptr,
        .targetCount = 1,
        .targets = &colorTarget,
    };

    wgpu::DepthStencilState depthStencilState = {
        .format = depthTextureFormat,
        .depthWriteEnabled = true,
        .depthCompare = wgpu::CompareFunction::Less,
        .stencilReadMask = 0,
        .stencilWriteMask = 0,
    };

    wgpu::RenderPipelineDescriptor pipelineDesc{
        .label = "rectangle",
        .vertex = wgpu::VertexState{
            .module = this->shaderModule->Get(),
            .entryPoint = "vs_main",
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = 2,
            .buffers = std::array{vertexBufferLayout, instanceBufferLayout}.data(),
        },
        .primitive = wgpu::PrimitiveState{
            .topology = wgpu::PrimitiveTopology::TriangleStrip,
            .stripIndexFormat = wgpu::IndexFormat::Undefined,
            .frontFace = wgpu::FrontFace::CCW,
            .cullMode = wgpu::CullMode::None,
        },
        .depthStencil = &depthStencilState,
        .multisample = wgpu::MultisampleState{
            .count = 1,
            .mask = ~0u,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragmentState,
    };

    wgpu::PipelineLayoutDescriptor layoutDesc{};
    pipelineDesc.layout = device->CreatePipelineLayout(&layoutDesc);

    this->pipeline = std::make_unique<wgpu::RenderPipeline>(device->CreateRenderPipeline(&pipelineDesc));

    return this->pipeline != nullptr;
}

bool RectangleShader::InitVertexBuffer(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue) {
    std::array<glm::vec3, 4> quadVertices = {
        glm::vec3(-1.0f, 1.0f, 0.0f),   // Top-left
        glm::vec3(-1.0f, -1.0f, 0.0f),  // Bottom-left
        glm::vec3(1.0f, 1.0f, 0.0f),    // Top-right
        glm::vec3(1.0f, -1.0f, 0.0f)    // Bottom-right
    };

    wgpu::BufferDescriptor bufferDesc{
        .label = "rectangle_vertex_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
        .size = quadVertices.size() * sizeof(glm::vec3),
        .mappedAtCreation = false,
    };

    this->vertexBuffer = std::make_unique<wgpu::Buffer>(device->CreateBuffer(&bufferDesc));
    if (this->vertexBuffer == nullptr) return false;

    queue->WriteBuffer(this->vertexBuffer->Get(), 0, quadVertices.data(), quadVertices.size() * sizeof(glm::vec3));

    return true;
}

bool RectangleShader::InitInstanceBuffer(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue) {
    wgpu::BufferDescriptor bufferDesc{
        .label = "rectangle_index_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
        .size = this->maxRectangleCount * sizeof(glm::mat4x4),
        .mappedAtCreation = false,
    };

    this->instanceBuffer = std::make_unique<wgpu::Buffer>(device->CreateBuffer(&bufferDesc));
    return this->instanceBuffer != nullptr;
}

void RectangleShader::Resize(const uint32_t width, const uint32_t height) {
    this->projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
}

bool RectangleShader::Init(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height) {
    if (!this->InitRenderPipeline(device, swapChainFormat, depthTextureFormat)) return false;
    if (!this->InitVertexBuffer(device, queue)) return false;
    if (!this->InitInstanceBuffer(device, queue)) return false;
    this->projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
    return true;
}

void RectangleShader::UpdateBuffers(const std::unique_ptr<wgpu::Queue> &queue, std::vector<glm::mat4x4> &instanceModelMatrices) {
    // projectionMatrix * viewMatrix * modelMatrix * position;
    // this needs to be done in the shader?
    this->viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 100.0f),  // Camera position in World Space
                                   glm::vec3(0.0f, 0.0, 0.0f),     // and looks at the origin
                                   glm::vec3(0.0f, 1.0f, 0.0f));   // Head is up

    for (auto &instanceModelMatrix : instanceModelMatrices) {
        instanceModelMatrix = this->projectionMatrix * this->viewMatrix * instanceModelMatrix;
    }

    queue->WriteBuffer(this->instanceBuffer->Get(), 0, instanceModelMatrices.data(), instanceModelMatrices.size() * sizeof(glm::mat4x4));
    this->instanceCount = instanceModelMatrices.size();
}

void RectangleShader::Render(const std::unique_ptr<wgpu::RenderPassEncoder> &renderPass, const std::unique_ptr<wgpu::Queue> &queue, float time) {
    renderPass->SetPipeline(this->pipeline->Get());
    renderPass->SetVertexBuffer(0, this->vertexBuffer->Get());
    renderPass->SetVertexBuffer(1, this->instanceBuffer->Get());

    renderPass->Draw(4, this->instanceCount, 0, 0);
}
