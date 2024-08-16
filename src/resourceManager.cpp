#include "resourceManager.hpp"
#include <fstream>

auto ResourceManager::LoadShaderModule(const std::filesystem::path& path, const wgpu::Device& device) -> std::unique_ptr<wgpu::ShaderModule> {
    std::ifstream file(path);
    std::string shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
    shaderCodeDesc.nextInChain = nullptr;
    shaderCodeDesc.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();
    wgpu::ShaderModuleDescriptor shaderDesc{
        .nextInChain = &shaderCodeDesc,
        .label = path.filename().c_str(),
    };

    return std::make_unique<wgpu::ShaderModule>(device.CreateShaderModule(&shaderDesc));
}