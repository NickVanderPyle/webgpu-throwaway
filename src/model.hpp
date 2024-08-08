#pragma once
#include <webgpu/webgpu_cpp.h>
#include <memory>
#include "glm/ext/vector_float3.hpp"

struct VertexAttributes {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

struct ModelData {
    std::unique_ptr<wgpu::Buffer> vertexBuffer;
    size_t vertexCount;
    std::unique_ptr<wgpu::Buffer> indexBuffer;
    size_t indexCount;

    ModelData(std::unique_ptr<wgpu::Buffer> vb, size_t vc, std::unique_ptr<wgpu::Buffer> ib, size_t ic)
        : vertexBuffer(std::move(vb)), vertexCount(vc), indexBuffer(std::move(ib)), indexCount(ic) {}

    ModelData(const ModelData&) = delete;
    ModelData& operator=(const ModelData&) = delete;
};

class Model {
   public:
    Model() = delete;
    ~Model() = delete;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(Model&&) = delete;

    static std::unique_ptr<ModelData> MakeModel(const std::unique_ptr<wgpu::Device>& device, const std::unique_ptr<wgpu::Queue>& queue);
};
