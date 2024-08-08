#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <memory>

class ResourceManager {
   public:
    ResourceManager() = delete;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    struct VertexAttributes {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
    };

    static std::unique_ptr<wgpu::ShaderModule> LoadShaderModule(const std::filesystem::path& path, const std::unique_ptr<wgpu::Device>& device);
};