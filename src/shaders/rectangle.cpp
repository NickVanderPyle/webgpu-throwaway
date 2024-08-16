#include "rectangle.hpp"
#include <cstddef>
#include "../resourceManager.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

// offsetof from std lib has trouble with VSCode intellisense, override.
template <typename T, typename U>
constexpr auto offsetOfMember(U T::*member) -> size_t {
    return (char *)&((T *)nullptr->*member) - (char *)nullptr;
}

struct VertexAttributes {
    glm::vec3 position;
    glm::vec3 color;
};

RectangleShader::RectangleShader(size_t maxRectangleCount) : maxRectangleCount(maxRectangleCount) {
}

auto RectangleShader::InitRenderPipeline(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) -> bool {
    this->shaderModule = ResourceManager::LoadShaderModule("/src/shaders/rectangle.wgsl", device);

    std::array<wgpu::VertexAttribute, 2> vertexAttribs{
        // VertexAttributes::positiom
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = offsetOfMember(&VertexAttributes::position),
            .shaderLocation = 0,
        },
        // VertexAttributes::color
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = offsetOfMember(&VertexAttributes::color),
            .shaderLocation = 1,
        },
    };
    wgpu::VertexBufferLayout vertexBufferLayout{
        .arrayStride = sizeof(VertexAttributes),
        .stepMode = wgpu::VertexStepMode::Vertex,
        .attributeCount = (uint32_t)vertexAttribs.size(),
        .attributes = vertexAttribs.data(),
    };

    // VertexAttributes::modelMatrix
    std::array<wgpu::VertexAttribute, 4> instanceAttribs{};
    for (size_t i = 0; auto &attrib : instanceAttribs) {
        attrib = wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x4,
            .offset = (uint64_t)(sizeof(glm::vec4) * i),  // glm::mat4x4, each row is vec4
            .shaderLocation = 2 + i,
        };
        i++;
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
    pipelineDesc.layout = device.CreatePipelineLayout(&layoutDesc);

    this->pipeline = std::make_unique<wgpu::RenderPipeline>(device.CreateRenderPipeline(&pipelineDesc));

    return this->pipeline != nullptr;
}

auto RectangleShader::InitVertexBuffer(const wgpu::Device &device, const wgpu::Queue &queue) -> bool {
    std::array<VertexAttributes, 4> quadVertices = {
        VertexAttributes{.position = glm::vec3(-1.0f, 1.0f, 0.0f), .color = glm::vec3(1.0f, 0.0f, 0.0f)},   // Top-left
        VertexAttributes{.position = glm::vec3(-1.0f, -1.0f, 0.0f), .color = glm::vec3(0.0f, 1.0f, 0.0f)},  // Bottom-left
        VertexAttributes{.position = glm::vec3(1.0f, 1.0f, 0.0f), .color = glm::vec3(0.0f, 0.0f, 1.0f)},    // Top-right
        VertexAttributes{.position = glm::vec3(1.0f, -1.0f, 0.0f), .color = glm::vec3(1.0f, 0.0f, 1.0f)}    // Bottom-right
    };

    wgpu::BufferDescriptor bufferDesc{
        .label = "rectangle_vertex_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
        .size = (uint64_t)(quadVertices.size() * sizeof(VertexAttributes)),
        .mappedAtCreation = false,
    };

    this->vertexBuffer = std::make_unique<wgpu::Buffer>(device.CreateBuffer(&bufferDesc));
    if (this->vertexBuffer == nullptr) {
        return false;
    }

    queue.WriteBuffer(this->vertexBuffer->Get(), 0, quadVertices.data(), quadVertices.size() * sizeof(VertexAttributes));

    return true;
}

auto RectangleShader::InitInstanceBuffer(const wgpu::Device &device) -> bool {
    wgpu::BufferDescriptor bufferDesc{
        .label = "rectangle_index_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
        .size = (uint64_t)(this->maxRectangleCount * sizeof(glm::mat4x4)),
        .mappedAtCreation = false,
    };

    this->instanceBuffer = std::make_unique<wgpu::Buffer>(device.CreateBuffer(&bufferDesc));
    return this->instanceBuffer != nullptr;
}

void RectangleShader::Resize(const uint32_t width, const uint32_t height) {
    this->projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
}

auto RectangleShader::Init(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool {
    this->projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
    return this->InitRenderPipeline(device, swapChainFormat, depthTextureFormat)
        && this->InitVertexBuffer(device, queue)
        && this->InitInstanceBuffer(device);
}

void RectangleShader::UpdateBuffers(const wgpu::Queue &queue, std::vector<glm::mat4x4> &instanceModelMatrices) {
    // projectionMatrix * viewMatrix * modelMatrix * position;
    // this needs to be done in the shader?
    this->viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 100.0f),  // Camera position in World Space
                                   glm::vec3(0.0f, 0.0, 0.0f),     // and looks at the origin
                                   glm::vec3(0.0f, 1.0f, 0.0f));   // Head is up

    for (auto &instanceModelMatrix : instanceModelMatrices) {
        instanceModelMatrix = this->projectionMatrix * this->viewMatrix * instanceModelMatrix;
    }

    queue.WriteBuffer(this->instanceBuffer->Get(), 0, instanceModelMatrices.data(), instanceModelMatrices.size() * sizeof(glm::mat4x4));
    this->instanceCount = instanceModelMatrices.size();
}

void RectangleShader::Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue & /*queue*/, const float /*time*/) {
    renderPass.SetPipeline(this->pipeline->Get());
    renderPass.SetVertexBuffer(0, this->vertexBuffer->Get());
    renderPass.SetVertexBuffer(1, this->instanceBuffer->Get());

    renderPass.Draw(4, this->instanceCount, 0, 0);
}
