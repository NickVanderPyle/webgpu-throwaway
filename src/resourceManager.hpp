#pragma once
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <filesystem>

class ResourceManager {
   public:
    ResourceManager() = delete;

    static auto LoadShaderModule(const std::filesystem::path& path, const wgpu::Device& device) -> std::unique_ptr<wgpu::ShaderModule>;
};