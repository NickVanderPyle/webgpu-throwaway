#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "shaders/line3d.hpp"
#include "shaders/rectangle.hpp"

class Graphics {
   public:
    Graphics();
    ~Graphics() = default;
    Graphics(const Graphics &) = delete;
    Graphics &operator=(const Graphics &) = delete;
    Graphics(Graphics &&) = delete;
    Graphics &operator=(Graphics &&) = delete;

    void DrawLine(const glm::vec3 start, const glm::vec3 end, const glm::vec3 color);
    void DrawRect(const glm::mat4x4 transform);
    // void DrawPolygon(int x, int y, const std::vector<glm::vec2> &vertices, glm::vec3 color);
    // void DrawCircle(int x, int y, int radius, float angle, glm::vec3 color);
    // void DrawFillCircle(int x, int y, int radius, glm::vec3 color);
    // void DrawFillRect(int x, int y, int width, int height, glm::vec3 color);
    // void DrawFillPolygon(int x, int y, const std::vector<glm::vec2> &vertices, glm::vec3 color);

    bool InitShaders(const std::unique_ptr<wgpu::Device> &device, const wgpu::TextureFormat swapChainFormat, const wgpu::TextureFormat depthTextureFormat, const std::unique_ptr<wgpu::Queue> &queue, const uint32_t width, const uint32_t height);
    void Resize(const uint32_t width, const uint32_t height);
    void Render(const std::unique_ptr<wgpu::RenderPassEncoder> &renderPass, const std::unique_ptr<wgpu::Queue> &queue, float time);

   private:
    std::unique_ptr<Line3DShader> line3d_shader;
    std::vector<Line3D> line3d_lines;
    const size_t line3d_maxLineCount = 5000;

    std::unique_ptr<RectangleShader> rectangle_shader;
    std::vector<glm::mat4x4> rectangle_instanceModelMatrices;
    const size_t rectangle_maxRectangleCount = 5000;
};
