#include "line3d.hpp"
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

Line3DShader::Line3DShader(size_t maxLineCount) : maxLineCount(maxLineCount) {}

auto Line3DShader::InitBindGroupLayout(const wgpu::Device &device) -> bool {
    std::array<wgpu::BindGroupLayoutEntry, 1> bindingLayoutEntries{
        wgpu::BindGroupLayoutEntry{
            .binding = 0,
            .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
            .buffer = wgpu::BufferBindingLayout{
                .type = wgpu::BufferBindingType::Uniform,
                .hasDynamicOffset = true,
                .minBindingSize = sizeof(MyUniforms),
            },
        },
    };

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{
        .label = "line3d",
        .entryCount = (uint32_t)bindingLayoutEntries.size(),
        .entries = bindingLayoutEntries.data(),
    };
    this->bindGroupLayout = std::make_unique<wgpu::BindGroupLayout>(device.CreateBindGroupLayout(&bindGroupLayoutDesc));

    return this->bindGroupLayout != nullptr;
}

auto Line3DShader::InitRenderPipeline(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) -> bool {
    this->shaderModule = ResourceManager::LoadShaderModule("/src/shaders/line3d.wgsl", device);

    std::array<wgpu::VertexAttribute, 2> vertexAttribs{
        // Position attribute
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = offsetOfMember(&VertexAttributes::position),
            .shaderLocation = 0,
        },
        // Color attribute
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
        .label = "line3d",
        .vertex = wgpu::VertexState{
            .module = this->shaderModule->Get(),
            .entryPoint = "vs_main",
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = 1,
            .buffers = &vertexBufferLayout,
        },
        .primitive = wgpu::PrimitiveState{
            .topology = wgpu::PrimitiveTopology::LineList,
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

    wgpu::PipelineLayoutDescriptor layoutDesc{
        .label = "line3d",
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = this->bindGroupLayout.get(),
    };

    pipelineDesc.layout = device.CreatePipelineLayout(&layoutDesc);

    this->pipeline = std::make_unique<wgpu::RenderPipeline>(device.CreateRenderPipeline(&pipelineDesc));

    return this->pipeline != nullptr;
}

auto Line3DShader::InitUniforms(const wgpu::Device &device, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool {
    wgpu::BufferDescriptor bufferDesc{
        .label = "line3d",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
        .size = sizeof(MyUniforms),
        .mappedAtCreation = false,
    };
    this->uniformBuffer = std::make_unique<wgpu::Buffer>(device.CreateBuffer(&bufferDesc));

    if (this->uniformBuffer == nullptr) {
        return false;
    }

    this->uniforms.modelMatrix = glm::mat4x4(1.0f);
    this->uniforms.viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f),  // Camera position in World Space
                                            glm::vec3(0.0f, 0.0, 0.0f),    // and looks at the origin
                                            glm::vec3(0.0f, 1.0f, 0.0f));  // Head is up
    this->uniforms.projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
    this->uniforms.time = 1.0f;
    queue.WriteBuffer(this->uniformBuffer->Get(), 0, &this->uniforms, sizeof(MyUniforms));

    return true;
}

auto Line3DShader::InitBindGroup(const wgpu::Device &device, const wgpu::Buffer &uniformBuffer, const wgpu::BindGroupLayout &bindGroupLayout) -> bool {
    std::array<wgpu::BindGroupEntry, 1> bindings = {
        wgpu::BindGroupEntry{
            .binding = 0,
            .buffer = uniformBuffer.Get(),
            .offset = 0,
            .size = sizeof(MyUniforms),
        },
    };

    wgpu::BindGroupDescriptor bindGroupDesc = {
        .layout = bindGroupLayout.Get(),
        .entryCount = (uint32_t)bindings.size(),
        .entries = bindings.data(),
    };
    this->bindGroup = std::make_unique<wgpu::BindGroup>(device.CreateBindGroup(&bindGroupDesc));

    return this->bindGroup != nullptr;
}

auto Line3DShader::InitVertexBuffer(const wgpu::Device &device) -> bool {
    wgpu::BufferDescriptor bufferDesc{
        .label = "line3d",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
        .size = (uint64_t)this->maxLineCount * sizeof(Line3D),
        .mappedAtCreation = false,
    };
    this->vertexBuffer = std::make_unique<wgpu::Buffer>(device.CreateBuffer(&bufferDesc));
    return this->vertexBuffer != nullptr;
}

void Line3DShader::Resize(const uint32_t width, const uint32_t height) {
    this->uniforms.projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
}

auto Line3DShader::Init(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue, const uint32_t width, const uint32_t height) -> bool {
    return this->InitBindGroupLayout(device)
        && this->InitRenderPipeline(device, swapChainFormat, depthTextureFormat)
        && this->InitUniforms(device, queue, width, height)
        && this->InitBindGroup(device, this->uniformBuffer->Get(), this->bindGroupLayout->Get())
        && this->InitVertexBuffer(device);
}

void Line3DShader::UpdateVertexBuffer(const wgpu::Queue &queue, const std::vector<Line3D> &lines) {
    queue.WriteBuffer(this->vertexBuffer->Get(), 0, lines.data(), lines.size() * sizeof(Line3D));
    this->drawLineCount = lines.size();
}

void Line3DShader::Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue &queue, const float time) {
    MyUniforms uniforms = this->uniforms;
    uniforms.time = time;

    auto angle = 20.0f * time;

    glm::mat4 rotationMatrix =
        glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *  // Rotate around Z axis
        glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) *  // Rotate around Y axis
        glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));    // Rotate around X axis

    this->uniforms.modelMatrix = rotationMatrix;  // glm::mat4x4(1.0f);

    renderPass.SetPipeline(this->pipeline->Get());
    renderPass.SetVertexBuffer(0, this->vertexBuffer->Get(), 0, (uint64_t)this->drawLineCount * sizeof(Line3D));

    uint32_t dynamicOffset = 0;
    queue.WriteBuffer(this->uniformBuffer->Get(), dynamicOffset, &uniforms, sizeof(MyUniforms));
    renderPass.SetBindGroup(0, this->bindGroup->Get(), 1, &dynamicOffset);
    renderPass.Draw(this->drawLineCount * 2, 1, 0, 0);
}