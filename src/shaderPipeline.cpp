#include "shaderPipeline.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "resourceManager.hpp"

bool ShaderPipeline::InitBindGroupLayout(const std::unique_ptr<wgpu::Device> &device) {
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
        .entryCount = (uint32_t)bindingLayoutEntries.size(),
        .entries = bindingLayoutEntries.data(),
    };
    this->bindGroupLayout = std::make_unique<wgpu::BindGroupLayout>(device->CreateBindGroupLayout(&bindGroupLayoutDesc));

    return this->bindGroupLayout != nullptr;
}

bool ShaderPipeline::InitRenderPipeline(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat) {
    this->shaderModule = ResourceManager::LoadShaderModule("/assets/shaders/vertex_shader.wgsl", device);

    std::array<wgpu::VertexAttribute, 3> vertexAttribs{
        // Position attribute
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = 0,
            .shaderLocation = 0,
        },
        // Normal attribute
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = offsetof(ResourceManager::VertexAttributes, normal),
            .shaderLocation = 1,
        },
        // Color attribute
        wgpu::VertexAttribute{
            .format = wgpu::VertexFormat::Float32x3,
            .offset = offsetof(ResourceManager::VertexAttributes, color),
            .shaderLocation = 2,
        },
    };

    wgpu::VertexBufferLayout vertexBufferLayout{
        .arrayStride = sizeof(ResourceManager::VertexAttributes),
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
        .vertex = wgpu::VertexState{
            .module = this->shaderModule->Get(),
            .entryPoint = "vs_main",
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = 1,
            .buffers = &vertexBufferLayout,
        },
        .primitive = wgpu::PrimitiveState{
            .topology = wgpu::PrimitiveTopology::TriangleList,
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
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = this->bindGroupLayout.get(),
    };

    pipelineDesc.layout = device->CreatePipelineLayout(&layoutDesc);

    this->pipeline = std::make_unique<wgpu::RenderPipeline>(device->CreateRenderPipeline(&pipelineDesc));

    return this->pipeline != nullptr;
}

bool ShaderPipeline::InitUniforms(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(MyUniforms);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    bufferDesc.mappedAtCreation = false;
    this->uniformBuffer = std::make_unique<wgpu::Buffer>(device->CreateBuffer(&bufferDesc));

    if (this->uniformBuffer == nullptr) {
        return false;
    }

    this->uniforms.modelMatrix = glm::mat4x4(1.0f);
    this->uniforms.viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f),  // Camera position in World Space
                                            glm::vec3(0.0f, 0.0, 0.0f),    // and looks at the origin
                                            glm::vec3(0.0f, 1.0f, 0.0f));  // Head is up
    this->uniforms.projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
    this->uniforms.time = 1.0f;
    queue->WriteBuffer(this->uniformBuffer->Get(), 0, &this->uniforms, sizeof(MyUniforms));

    return true;
}

bool ShaderPipeline::InitBindGroup(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Buffer> &uniformBuffer, const std::unique_ptr<wgpu::BindGroupLayout> &bindGroupLayout) {
    std::array<wgpu::BindGroupEntry, 1> bindings = {
        wgpu::BindGroupEntry{
            .binding = 0,
            .buffer = uniformBuffer->Get(),
            .offset = 0,
            .size = sizeof(MyUniforms),
        },
    };

    wgpu::BindGroupDescriptor bindGroupDesc = {
        .layout = bindGroupLayout->Get(),
        .entryCount = (uint32_t)bindings.size(),
        .entries = bindings.data(),
    };
    this->bindGroup = std::make_unique<wgpu::BindGroup>(device->CreateBindGroup(&bindGroupDesc));

    return this->bindGroup != nullptr;
}

void ShaderPipeline::Resize(const uint32_t width, const uint32_t height) {
    this->uniforms.projectionMatrix = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 1000.0f);
}

bool ShaderPipeline::Init(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height) {
    if (!this->InitBindGroupLayout(device)) return false;
    if (!this->InitRenderPipeline(device, swapChainFormat, depthTextureFormat)) return false;
    if (!this->InitUniforms(device, queue, width, height)) return false;
    if (!this->InitBindGroup(device, this->uniformBuffer, this->bindGroupLayout)) return false;

    this->model = Model::MakeModel(device, queue);

    return true;
}

void ShaderPipeline::Render(const std::unique_ptr<wgpu::RenderPassEncoder> &renderPass, const std::unique_ptr<wgpu::Queue> &queue, float time) {
    MyUniforms uniforms = this->uniforms;
    uniforms.time = time;

    auto angle = 20.0f * time;

    glm::mat4 rotationMatrix =
        glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *  // Rotate around Z axis
        glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) *  // Rotate around Y axis
        glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));    // Rotate around X axis

    this->uniforms.modelMatrix = rotationMatrix;  // glm::mat4x4(1.0f);

    renderPass->SetPipeline(this->pipeline->Get());

    renderPass->SetVertexBuffer(0, this->model->vertexBuffer->Get(), 0, this->model->vertexCount * sizeof(VertexAttributes));
    renderPass->SetIndexBuffer(this->model->indexBuffer->Get(), wgpu::IndexFormat::Uint32, 0, this->model->indexCount * sizeof(uint32_t));

    uint32_t dynamicOffset = 0;
    queue->WriteBuffer(this->uniformBuffer->Get(), dynamicOffset, &uniforms, sizeof(MyUniforms));
    renderPass->SetBindGroup(0, this->bindGroup->Get(), 1, &dynamicOffset);
    renderPass->DrawIndexed(this->model->indexCount);
}