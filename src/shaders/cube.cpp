#include "cube.hpp"
#include <cstddef>
#include <vector>
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

CubeShader::CubeShader(size_t maxCubeCount) : maxCubeCount(maxCubeCount) {
}

auto CubeShader::InitRenderPipeline(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) -> bool {
    this->shaderModule = ResourceManager::LoadShaderModule("/src/shaders/cube.wgsl", device);

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
        .label = "cube",
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

    wgpu::PipelineLayoutDescriptor layoutDesc{
        .label = "cube pipeline layout",
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = this->bindGroupLayout.get(),
    };

    pipelineDesc.layout = device.CreatePipelineLayout(&layoutDesc);

    this->pipeline = std::make_unique<wgpu::RenderPipeline>(device.CreateRenderPipeline(&pipelineDesc));

    return this->pipeline != nullptr;
}

auto CubeShader::InitUniforms(const wgpu::Device &device) -> bool {
    wgpu::BufferDescriptor bufferDesc{
        .label = "cube",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
        .size = sizeof(MyUniforms),
        .mappedAtCreation = false,
    };
    this->uniformBuffer = std::make_unique<wgpu::Buffer>(device.CreateBuffer(&bufferDesc));

    return this->uniformBuffer != nullptr;
}

auto CubeShader::InitBindGroupLayout(const wgpu::Device &device) -> bool {
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
        .label = "cube",
        .entryCount = (uint32_t)bindingLayoutEntries.size(),
        .entries = bindingLayoutEntries.data(),
    };
    this->bindGroupLayout = std::make_unique<wgpu::BindGroupLayout>(device.CreateBindGroupLayout(&bindGroupLayoutDesc));

    return this->bindGroupLayout != nullptr;
}

auto CubeShader::InitBindGroup(const wgpu::Device &device, const wgpu::Buffer &uniformBuffer, const wgpu::BindGroupLayout &bindGroupLayout) -> bool {
    std::array<wgpu::BindGroupEntry, 1> bindings = {
        wgpu::BindGroupEntry{
            .binding = 0,
            .buffer = uniformBuffer.Get(),
            .offset = 0,
            .size = sizeof(MyUniforms),
        },
    };

    wgpu::BindGroupDescriptor bindGroupDesc = {
        .label = "cube uniform bind group",
        .layout = bindGroupLayout.Get(),
        .entryCount = (uint32_t)bindings.size(),
        .entries = bindings.data(),
    };
    this->bindGroup = std::make_unique<wgpu::BindGroup>(device.CreateBindGroup(&bindGroupDesc));

    return this->bindGroup != nullptr;
}

auto CubeShader::InitVertexBuffer(const wgpu::Device &device, const wgpu::Queue &queue) -> bool {
    std::vector<VertexAttributes> quadVertices = {
        VertexAttributes{.position = glm::vec3(-1.0f, 1.0f, 1.0f), .color = glm::vec3(1.0f, 0.0f, 0.0f)},    // Front-top-left
        VertexAttributes{.position = glm::vec3(1.0f, 1.0f, 1.0f), .color = glm::vec3(0.0f, 1.0f, 0.0f)},     // Front-top-right
        VertexAttributes{.position = glm::vec3(-1.0f, -1.0f, 1.0f), .color = glm::vec3(0.0f, 0.0f, 1.0f)},   // Front-bottom-left
        VertexAttributes{.position = glm::vec3(1.0f, -1.0f, 1.0f), .color = glm::vec3(1.0f, 0.0f, 1.0f)},    // Front-bottom-right
        VertexAttributes{.position = glm::vec3(1.0f, -1.0f, -1.0f), .color = glm::vec3(1.0f, 1.0f, 0.0f)},   // Back-bottom-right
        VertexAttributes{.position = glm::vec3(1.0f, 1.0f, 1.0f), .color = glm::vec3(0.0f, 1.0f, 0.0f)},     // Front-top-right
        VertexAttributes{.position = glm::vec3(1.0f, 1.0f, -1.0f), .color = glm::vec3(1.0f, 0.0f, 1.0f)},    // Back-top-right
        VertexAttributes{.position = glm::vec3(-1.0f, 1.0f, 1.0f), .color = glm::vec3(1.0f, 0.0f, 0.0f)},    // Front-top-left
        VertexAttributes{.position = glm::vec3(-1.0f, 1.0f, -1.0f), .color = glm::vec3(1.0f, 0.0f, 1.0f)},   // Back-top-left
        VertexAttributes{.position = glm::vec3(-1.0f, -1.0f, 1.0f), .color = glm::vec3(0.0f, 0.0f, 1.0f)},   // Front-bottom-left
        VertexAttributes{.position = glm::vec3(-1.0f, -1.0f, -1.0f), .color = glm::vec3(0.5f, 0.5f, 0.5f)},  // Back-bottom-left
        VertexAttributes{.position = glm::vec3(1.0f, -1.0f, -1.0f), .color = glm::vec3(1.0f, 1.0f, 0.0f)},   // Back-bottom-right
        VertexAttributes{.position = glm::vec3(-1.0f, 1.0f, -1.0f), .color = glm::vec3(1.0f, 0.0f, 1.0f)},   // Back-top-left
        VertexAttributes{.position = glm::vec3(1.0f, 1.0f, -1.0f), .color = glm::vec3(1.0f, 0.0f, 1.0f)},    // Back-top-right
    };

    wgpu::BufferDescriptor bufferDesc{
        .label = "cube_vertex_buffer",
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

auto CubeShader::InitInstanceBuffer(const wgpu::Device &device) -> bool {
    wgpu::BufferDescriptor bufferDesc{
        .label = "cube_index_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
        .size = (uint64_t)(this->maxCubeCount * sizeof(glm::mat4x4)),
        .mappedAtCreation = false,
    };

    this->instanceBuffer = std::make_unique<wgpu::Buffer>(device.CreateBuffer(&bufferDesc));
    return this->instanceBuffer != nullptr;
}

auto CubeShader::Init(const wgpu::Device &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const wgpu::Queue &queue) -> bool {
    return this->InitBindGroupLayout(device)
        && this->InitRenderPipeline(device, swapChainFormat, depthTextureFormat)
        && this->InitUniforms(device)
        && this->InitBindGroup(device, this->uniformBuffer->Get(), this->bindGroupLayout->Get())
        && this->InitVertexBuffer(device, queue)
        && this->InitInstanceBuffer(device);
}

void CubeShader::UpdateBuffers(const wgpu::Queue &queue, std::vector<glm::mat4x4> &instanceModelMatrices) {
    queue.WriteBuffer(this->instanceBuffer->Get(), 0, instanceModelMatrices.data(), instanceModelMatrices.size() * sizeof(glm::mat4x4));
    this->instanceCount = instanceModelMatrices.size();
}

void CubeShader::Render(const wgpu::RenderPassEncoder &renderPass, const wgpu::Queue &queue, const glm::mat4x4 cameraViewMatrix, const glm::mat4x4 projectionMatrix, const float time) {
    MyUniforms uniforms = this->uniforms;
    uniforms.time = time;

    this->uniforms.viewMatrix = cameraViewMatrix;
    this->uniforms.projectionMatrix = projectionMatrix;

    renderPass.SetPipeline(this->pipeline->Get());
    renderPass.SetVertexBuffer(0, this->vertexBuffer->Get());
    renderPass.SetVertexBuffer(1, this->instanceBuffer->Get());

    uint32_t dynamicOffset = 0;
    queue.WriteBuffer(this->uniformBuffer->Get(), dynamicOffset, &uniforms, sizeof(MyUniforms));
    renderPass.SetBindGroup(0, this->bindGroup->Get(), 1, &dynamicOffset);

    renderPass.Draw(14, this->instanceCount, 0, 0);
}
