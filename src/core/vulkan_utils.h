#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace vkdemo
{

// Forward declaration
class VulkanContext;

//=============================================================================
// Common Data Structures
//=============================================================================

// Queue family indices
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

// Swapchain support details
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Render target wrapper (reusable)
struct RenderTarget
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t width = 0;
    uint32_t height = 0;

    void Cleanup(VkDevice device);
};

// Pipeline configuration (reusable)
struct PipelineConfig
{
    std::string vertShaderPath;
    std::string fragShaderPath;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    uint32_t colorAttachmentCount = 1;
    bool hasDepthAttachment = false;
    bool isFullscreenQuad = false;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    bool enableBlending = false;
};

// Fullscreen quad vertex
struct FullscreenVertex
{
    glm::vec2 position;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
};

//=============================================================================
// Utility Functions
//=============================================================================

namespace utils
{

// File reading
std::vector<char> ReadFile(const std::string& filename);

// Pipeline creation helper
VkPipeline CreatePipeline(VulkanContext& ctx, const PipelineConfig& config);

// Sampler creation helpers
VkSampler CreateLinearSampler(VkDevice device);
VkSampler CreateNearestSampler(VkDevice device);

}  // namespace utils

//=============================================================================
// Fullscreen Quad Manager
//=============================================================================

class FullscreenQuad
{
public:
    void Initialize(VulkanContext& ctx);
    void Cleanup(VkDevice device);

    void Bind(VkCommandBuffer cmd) const;
    void Draw(VkCommandBuffer cmd) const;

    bool IsInitialized() const { return initialized; }

private:
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    bool initialized = false;
};

}  // namespace vkdemo
