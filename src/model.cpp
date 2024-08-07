#include "model.hpp"
#include <webgpu/webgpu_cpp.h>
#include <memory>

std::unique_ptr<ModelData> Model::MakeModel(const std::unique_ptr<wgpu::Device> &device, const std::unique_ptr<wgpu::Queue> &queue) {
    std::array<VertexAttributes, 4> vertexData = {
        VertexAttributes{{-5.0f, -5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left, Red
        VertexAttributes{{-5.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},   // Top-left, Blue
        VertexAttributes{{5.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},    // Top-right, Yellow
        VertexAttributes{{5.0f, -5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},   // Bottom-right, Green
    };
    std::array<uint32_t, 6> indices = {
        0, 1, 2,
        0, 2, 3};

    // Upload vertex buffer.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = vertexData.size() * sizeof(VertexAttributes);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
    bufferDesc.mappedAtCreation = false;
    auto vertexBuffer = std::make_unique<wgpu::Buffer>(device->CreateBuffer(&bufferDesc));
    queue->WriteBuffer(vertexBuffer->Get(), 0, vertexData.data(), bufferDesc.size);

    // Upload index buffer.
    bufferDesc.size = indices.size() * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
    auto indexBuffer = std::make_unique<wgpu::Buffer>(device->CreateBuffer(&bufferDesc));
    queue->WriteBuffer(indexBuffer->Get(), 0, indices.data(), bufferDesc.size);

    return std::make_unique<ModelData>(std::move(vertexBuffer), vertexData.size(), std::move(indexBuffer), indices.size());
}