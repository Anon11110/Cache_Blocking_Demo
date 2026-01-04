#pragma once

#include "../../core/vulkan_utils.h"
#include "../example_base.h"

#include <array>
#include <glm/glm.hpp>

namespace vkdemo
{

struct MotionBlurMVPUBO
{
    alignas(16) glm::mat4 currMVP;
    alignas(16) glm::mat4 prevMVP;
};

struct MotionBlurPostProcessParams
{
    alignas(4) float blurStrength;
    alignas(4) float motionScale;
    alignas(8) glm::vec2 texelSize;
};

struct TriangleVertex
{
    glm::vec3 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription GetBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
};

class MotionBlurExample : public ExampleBase
{
public:
    explicit MotionBlurExample(VulkanContext& context);
    ~MotionBlurExample() override = default;

    std::string GetName() const override { return "Motion Blur Demo"; }

    void Initialize() override;
    void Cleanup() override;
    void OnSwapChainRecreated() override;
    void OnSwapChainCleanup() override;
    void RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex) override;
    void Update(float deltaTime) override;

private:
    void CreateRenderTargets();
    void CreateRenderPasses();
    void CreateDescriptorSetLayouts();
    void CreatePipelineLayouts();
    void CreatePipelines();
    void CreateFramebuffers();
    void CreateTriangleVertexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateSamplers();

    void CleanupRenderTargets();
    void CleanupFramebuffers();

    // Render targets
    RenderTarget rtSceneColor;
    RenderTarget rtVelocity;
    RenderTarget rtDepth;
    RenderTarget rtMotion;
    RenderTarget rtBlurIntermediate;
    RenderTarget rtBlurFinal;

    // Framebuffers
    VkFramebuffer fbGBuffer = VK_NULL_HANDLE;
    VkFramebuffer fbMotionApply = VK_NULL_HANDLE;
    VkFramebuffer fbBlurVertical = VK_NULL_HANDLE;
    VkFramebuffer fbBlurHorizontal = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // Render passes
    VkRenderPass renderPassGBuffer = VK_NULL_HANDLE;
    VkRenderPass renderPassMotionApply = VK_NULL_HANDLE;
    VkRenderPass renderPassBlurVertical = VK_NULL_HANDLE;
    VkRenderPass renderPassBlurHorizontal = VK_NULL_HANDLE;
    VkRenderPass renderPassFinal = VK_NULL_HANDLE;

    // Descriptor set layouts
    VkDescriptorSetLayout descriptorSetLayoutGBuffer = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayoutPostProcess = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayoutFinal = VK_NULL_HANDLE;

    // Pipeline layouts
    VkPipelineLayout pipelineLayoutGBuffer = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayoutMotionApply = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayoutBlur = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayoutFinal = VK_NULL_HANDLE;

    // Pipelines
    VkPipeline pipelineGBuffer = VK_NULL_HANDLE;
    VkPipeline pipelineMotionApply = VK_NULL_HANDLE;
    VkPipeline pipelineBlurVertical = VK_NULL_HANDLE;
    VkPipeline pipelineBlurHorizontal = VK_NULL_HANDLE;
    VkPipeline pipelineFinal = VK_NULL_HANDLE;

    // Triangle mesh
    VkBuffer triangleVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory triangleVertexBufferMemory = VK_NULL_HANDLE;

    // Uniform buffers
    std::vector<VkBuffer> mvpUniformBuffers;
    std::vector<VkDeviceMemory> mvpUniformBuffersMemory;
    std::vector<void*> mvpUniformBuffersMapped;
    std::vector<VkBuffer> postProcessUniformBuffers;
    std::vector<VkDeviceMemory> postProcessUniformBuffersMemory;
    std::vector<void*> postProcessUniformBuffersMapped;

    // Descriptors
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSetsGBuffer;
    std::vector<VkDescriptorSet> descriptorSetsMotionApply;
    std::vector<VkDescriptorSet> descriptorSetsBlurVertical;
    std::vector<VkDescriptorSet> descriptorSetsBlurHorizontal;
    std::vector<VkDescriptorSet> descriptorSetsFinal;

    // Samplers
    VkSampler samplerLinear = VK_NULL_HANDLE;
    VkSampler samplerNearest = VK_NULL_HANDLE;

    // Fullscreen quad
    FullscreenQuad fullscreenQuad;

    // Animation state
    float totalTime = 0.0f;
    glm::mat4 previousMVP = glm::mat4(1.0f);
};

}  // namespace vkdemo
