#include "resourceManager.hpp"
#include <fstream>
#include <memory>
#include <string>

std::unique_ptr<wgpu::ShaderModule> ResourceManager::LoadShaderModule(const std::filesystem::path& path, const std::unique_ptr<wgpu::Device>& device) {
    std::ifstream file(path);
    std::string shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
    shaderCodeDesc.nextInChain = nullptr;
    shaderCodeDesc.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();
    wgpu::ShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &shaderCodeDesc;

    return std::make_unique<wgpu::ShaderModule>(device->CreateShaderModule(&shaderDesc));
}