#include "motion_blur_example.h"

#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

namespace vkdemo
{

static const std::vector<TriangleVertex> triangleVertices = {{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                                             {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                                             {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

VkVertexInputBindingDescription TriangleVertex::GetBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(TriangleVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> TriangleVertex::GetAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(TriangleVertex, position);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(TriangleVertex, color);
    return attributeDescriptions;
}

MotionBlurExample::MotionBlurExample(VulkanContext& context) : ExampleBase(context) {}

void MotionBlurExample::Initialize()
{
    CreateSamplers();
    CreateRenderTargets();
    CreateRenderPasses();
    CreateDescriptorSetLayouts();
    CreatePipelineLayouts();
    CreatePipelines();
    CreateFramebuffers();
    CreateTriangleVertexBuffer();
    fullscreenQuad.Initialize(ctx);
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
}

void MotionBlurExample::Cleanup()
{
    VkDevice device = ctx.GetDevice();

    // Cleanup samplers
    vkDestroySampler(device, samplerLinear, nullptr);
    vkDestroySampler(device, samplerNearest, nullptr);

    // Cleanup render targets
    rtSceneColor.Cleanup(device);
    rtVelocity.Cleanup(device);
    rtDepth.Cleanup(device);
    rtMotion.Cleanup(device);
    rtBlurIntermediate.Cleanup(device);
    rtBlurFinal.Cleanup(device);

    // Cleanup framebuffers
    CleanupFramebuffers();

    // Cleanup pipelines
    vkDestroyPipeline(device, pipelineGBuffer, nullptr);
    vkDestroyPipeline(device, pipelineMotionApply, nullptr);
    vkDestroyPipeline(device, pipelineBlurVertical, nullptr);
    vkDestroyPipeline(device, pipelineBlurHorizontal, nullptr);
    vkDestroyPipeline(device, pipelineFinal, nullptr);

    // Cleanup pipeline layouts
    vkDestroyPipelineLayout(device, pipelineLayoutGBuffer, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayoutMotionApply, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayoutBlur, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayoutFinal, nullptr);

    // Cleanup render passes
    vkDestroyRenderPass(device, renderPassGBuffer, nullptr);
    vkDestroyRenderPass(device, renderPassMotionApply, nullptr);
    vkDestroyRenderPass(device, renderPassBlurVertical, nullptr);
    vkDestroyRenderPass(device, renderPassBlurHorizontal, nullptr);
    vkDestroyRenderPass(device, renderPassFinal, nullptr);

    // Cleanup descriptor set layouts
    vkDestroyDescriptorSetLayout(device, descriptorSetLayoutGBuffer, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayoutPostProcess, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayoutFinal, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    // Cleanup uniform buffers
    for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, mvpUniformBuffers[i], nullptr);
        vkFreeMemory(device, mvpUniformBuffersMemory[i], nullptr);
        vkDestroyBuffer(device, postProcessUniformBuffers[i], nullptr);
        vkFreeMemory(device, postProcessUniformBuffersMemory[i], nullptr);
    }

    // Cleanup vertex buffer
    vkDestroyBuffer(device, triangleVertexBuffer, nullptr);
    vkFreeMemory(device, triangleVertexBufferMemory, nullptr);

    // Cleanup fullscreen quad
    fullscreenQuad.Cleanup(device);
}

void MotionBlurExample::OnSwapChainRecreated()
{
    CleanupRenderTargets();
    CreateRenderTargets();
    CreateFramebuffers();

    vkResetDescriptorPool(ctx.GetDevice(), descriptorPool, 0);
    CreateDescriptorSets();
}

void MotionBlurExample::OnSwapChainCleanup()
{
    CleanupFramebuffers();
}

void MotionBlurExample::CreateSamplers()
{
    samplerLinear = utils::CreateLinearSampler(ctx.GetDevice());
    samplerNearest = utils::CreateNearestSampler(ctx.GetDevice());
}

void MotionBlurExample::CreateRenderTargets()
{
    VkExtent2D extent = ctx.GetSwapChainExtent();

    // Scene Color
    rtSceneColor.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    rtSceneColor.width = extent.width;
    rtSceneColor.height = extent.height;
    ctx.CreateImage(extent.width, extent.height, rtSceneColor.format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rtSceneColor.image, rtSceneColor.memory);
    rtSceneColor.view = ctx.CreateImageView(rtSceneColor.image, rtSceneColor.format, VK_IMAGE_ASPECT_COLOR_BIT);

    // Velocity
    rtVelocity.format = VK_FORMAT_R16G16_SFLOAT;
    rtVelocity.width = extent.width;
    rtVelocity.height = extent.height;
    ctx.CreateImage(extent.width, extent.height, rtVelocity.format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rtVelocity.image, rtVelocity.memory);
    rtVelocity.view = ctx.CreateImageView(rtVelocity.image, rtVelocity.format, VK_IMAGE_ASPECT_COLOR_BIT);

    // Depth
    rtDepth.format = ctx.FindDepthFormat();
    rtDepth.width = extent.width;
    rtDepth.height = extent.height;
    ctx.CreateImage(extent.width, extent.height, rtDepth.format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rtDepth.image, rtDepth.memory);
    rtDepth.view = ctx.CreateImageView(rtDepth.image, rtDepth.format, VK_IMAGE_ASPECT_DEPTH_BIT);

    // Motion output
    rtMotion.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    rtMotion.width = extent.width;
    rtMotion.height = extent.height;
    ctx.CreateImage(extent.width, extent.height, rtMotion.format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rtMotion.image, rtMotion.memory);
    rtMotion.view = ctx.CreateImageView(rtMotion.image, rtMotion.format, VK_IMAGE_ASPECT_COLOR_BIT);

    // Blur Intermediate
    rtBlurIntermediate.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    rtBlurIntermediate.width = extent.width;
    rtBlurIntermediate.height = extent.height;
    ctx.CreateImage(extent.width, extent.height, rtBlurIntermediate.format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rtBlurIntermediate.image, rtBlurIntermediate.memory);
    rtBlurIntermediate.view =
        ctx.CreateImageView(rtBlurIntermediate.image, rtBlurIntermediate.format, VK_IMAGE_ASPECT_COLOR_BIT);

    // Blur Final
    rtBlurFinal.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    rtBlurFinal.width = extent.width;
    rtBlurFinal.height = extent.height;
    ctx.CreateImage(extent.width, extent.height, rtBlurFinal.format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rtBlurFinal.image, rtBlurFinal.memory);
    rtBlurFinal.view = ctx.CreateImageView(rtBlurFinal.image, rtBlurFinal.format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void MotionBlurExample::CleanupRenderTargets()
{
    VkDevice device = ctx.GetDevice();
    rtSceneColor.Cleanup(device);
    rtVelocity.Cleanup(device);
    rtDepth.Cleanup(device);
    rtMotion.Cleanup(device);
    rtBlurIntermediate.Cleanup(device);
    rtBlurFinal.Cleanup(device);
}

void MotionBlurExample::CreateRenderPasses()
{
    VkDevice device = ctx.GetDevice();

    // G-Buffer (SceneColor + Velocity + Depth)
    {
        std::array<VkAttachmentDescription, 3> attachments{};

        // SceneColor
        attachments[0].format = rtSceneColor.format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Velocity
        attachments[1].format = rtVelocity.format;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Depth
        attachments[2].format = rtDepth.format;
        attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        std::array<VkAttachmentReference, 2> colorRefs{};
        colorRefs[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorRefs[1] = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkAttachmentReference depthRef{};
        depthRef.attachment = 2;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = &depthRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPassGBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create G-Buffer render pass!");
        }
    }

    // Post-process passes
    auto createPostProcessRenderPass = [device](VkFormat format, VkRenderPass& renderPass)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create post-process render pass!");
        }
    };

    createPostProcessRenderPass(rtMotion.format, renderPassMotionApply);
    createPostProcessRenderPass(rtBlurIntermediate.format, renderPassBlurVertical);
    createPostProcessRenderPass(rtBlurFinal.format, renderPassBlurHorizontal);

    // Final pass
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = ctx.GetSwapChainFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPassFinal) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create final render pass!");
        }
    }
}

void MotionBlurExample::CreateDescriptorSetLayouts()
{
    VkDevice device = ctx.GetDevice();

    // G-Buffer layout (UBO only)
    {
        VkDescriptorSetLayoutBinding uboBinding{};
        uboBinding.binding = 0;
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayoutGBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create G-Buffer descriptor set layout!");
        }
    }

    // Post-process layout (samplers for input texture, velocity, depth)
    {
        std::array<VkDescriptorSetLayoutBinding, 4> bindings{};

        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[2].binding = 2;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[3].binding = 3;
        bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[3].descriptorCount = 1;
        bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayoutPostProcess) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create post-process descriptor set layout!");
        }
    }

    // Final pass layout (two samplers: motion result and blur result)
    {
        std::array<VkDescriptorSetLayoutBinding, 2> bindings{};

        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayoutFinal) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create final descriptor set layout!");
        }
    }
}

void MotionBlurExample::CreatePipelineLayouts()
{
    VkDevice device = ctx.GetDevice();

    // G-Buffer pipeline layout
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorSetLayoutGBuffer;

        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayoutGBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create G-Buffer pipeline layout!");
        }
    }

    // Motion Apply pipeline layout
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorSetLayoutPostProcess;

        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayoutMotionApply) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create motion apply pipeline layout!");
        }
    }

    // Blur pipeline layout
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorSetLayoutPostProcess;

        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayoutBlur) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create blur pipeline layout!");
        }
    }

    // Final pipeline layout
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorSetLayoutFinal;

        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayoutFinal) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create final pipeline layout!");
        }
    }
}

void MotionBlurExample::CreatePipelines()
{
    // G-Buffer pipeline
    {
        auto vertShaderCode = utils::ReadFile("shaders/gbuffer.vert.spv");
        auto fragShaderCode = utils::ReadFile("shaders/gbuffer.frag.spv");

        VkShaderModule vertShaderModule = ctx.CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = ctx.CreateShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        auto bindingDescription = TriangleVertex::GetBindingDescription();
        auto attributeDescriptions = TriangleVertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkExtent2D extent = ctx.GetSwapChainExtent();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        std::array<VkPipelineColorBlendAttachmentState, 2> colorBlendAttachments{};
        for (auto& attachment : colorBlendAttachments)
        {
            attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                        VK_COLOR_COMPONENT_A_BIT;
            attachment.blendEnable = VK_FALSE;
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayoutGBuffer;
        pipelineInfo.renderPass = renderPassGBuffer;
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(ctx.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineGBuffer) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create G-Buffer pipeline!");
        }

        vkDestroyShaderModule(ctx.GetDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(ctx.GetDevice(), vertShaderModule, nullptr);
    }

    // Post-process pipelines
    PipelineConfig configMotion{};
    configMotion.vertShaderPath = "shaders/motion_apply.vert.spv";
    configMotion.fragShaderPath = "shaders/motion_apply.frag.spv";
    configMotion.renderPass = renderPassMotionApply;
    configMotion.pipelineLayout = pipelineLayoutMotionApply;
    configMotion.colorAttachmentCount = 1;
    configMotion.hasDepthAttachment = false;
    configMotion.isFullscreenQuad = true;
    pipelineMotionApply = utils::CreatePipeline(ctx, configMotion);

    PipelineConfig configBlurV{};
    configBlurV.vertShaderPath = "shaders/blur_vertical.vert.spv";
    configBlurV.fragShaderPath = "shaders/blur_vertical.frag.spv";
    configBlurV.renderPass = renderPassBlurVertical;
    configBlurV.pipelineLayout = pipelineLayoutBlur;
    configBlurV.colorAttachmentCount = 1;
    configBlurV.hasDepthAttachment = false;
    configBlurV.isFullscreenQuad = true;
    pipelineBlurVertical = utils::CreatePipeline(ctx, configBlurV);

    PipelineConfig configBlurH{};
    configBlurH.vertShaderPath = "shaders/blur_horizontal.vert.spv";
    configBlurH.fragShaderPath = "shaders/blur_horizontal.frag.spv";
    configBlurH.renderPass = renderPassBlurHorizontal;
    configBlurH.pipelineLayout = pipelineLayoutBlur;
    configBlurH.colorAttachmentCount = 1;
    configBlurH.hasDepthAttachment = false;
    configBlurH.isFullscreenQuad = true;
    pipelineBlurHorizontal = utils::CreatePipeline(ctx, configBlurH);

    PipelineConfig configFinal{};
    configFinal.vertShaderPath = "shaders/final_apply.vert.spv";
    configFinal.fragShaderPath = "shaders/final_apply.frag.spv";
    configFinal.renderPass = renderPassFinal;
    configFinal.pipelineLayout = pipelineLayoutFinal;
    configFinal.colorAttachmentCount = 1;
    configFinal.hasDepthAttachment = false;
    configFinal.isFullscreenQuad = true;
    pipelineFinal = utils::CreatePipeline(ctx, configFinal);
}

void MotionBlurExample::CreateFramebuffers()
{
    VkDevice device = ctx.GetDevice();
    VkExtent2D extent = ctx.GetSwapChainExtent();
    const auto& swapChainImageViews = ctx.GetSwapChainImageViews();

    // Swapchain framebuffers for final pass
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassFinal;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create swapchain framebuffer!");
        }
    }

    // G-Buffer framebuffer
    {
        std::array<VkImageView, 3> attachments = {rtSceneColor.view, rtVelocity.view, rtDepth.view};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassGBuffer;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &fbGBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create G-Buffer framebuffer!");
        }
    }

    // Motion Apply framebuffer
    {
        VkImageView attachments[] = {rtMotion.view};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassMotionApply;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &fbMotionApply) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create motion apply framebuffer!");
        }
    }

    // Blur Vertical framebuffer
    {
        VkImageView attachments[] = {rtBlurIntermediate.view};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassBlurVertical;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &fbBlurVertical) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create blur vertical framebuffer!");
        }
    }

    // Blur Horizontal framebuffer
    {
        VkImageView attachments[] = {rtBlurFinal.view};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassBlurHorizontal;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &fbBlurHorizontal) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create blur horizontal framebuffer!");
        }
    }
}

void MotionBlurExample::CleanupFramebuffers()
{
    VkDevice device = ctx.GetDevice();

    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    swapChainFramebuffers.clear();

    if (fbGBuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, fbGBuffer, nullptr);
        fbGBuffer = VK_NULL_HANDLE;
    }
    if (fbMotionApply != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, fbMotionApply, nullptr);
        fbMotionApply = VK_NULL_HANDLE;
    }
    if (fbBlurVertical != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, fbBlurVertical, nullptr);
        fbBlurVertical = VK_NULL_HANDLE;
    }
    if (fbBlurHorizontal != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, fbBlurHorizontal, nullptr);
        fbBlurHorizontal = VK_NULL_HANDLE;
    }
}

void MotionBlurExample::CreateTriangleVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(triangleVertices[0]) * triangleVertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    ctx.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                     stagingBufferMemory);

    void* data;
    vkMapMemory(ctx.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, triangleVertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(ctx.GetDevice(), stagingBufferMemory);

    ctx.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, triangleVertexBuffer, triangleVertexBufferMemory);

    ctx.CopyBuffer(stagingBuffer, triangleVertexBuffer, bufferSize);

    vkDestroyBuffer(ctx.GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(ctx.GetDevice(), stagingBufferMemory, nullptr);
}

void MotionBlurExample::CreateUniformBuffers()
{
    VkDeviceSize mvpBufferSize = sizeof(MotionBlurMVPUBO);
    VkDeviceSize postProcessBufferSize = sizeof(MotionBlurPostProcessParams);

    mvpUniformBuffers.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
    mvpUniformBuffersMemory.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
    mvpUniformBuffersMapped.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
    postProcessUniformBuffers.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
    postProcessUniformBuffersMemory.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
    postProcessUniformBuffersMapped.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
    {
        ctx.CreateBuffer(mvpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         mvpUniformBuffers[i], mvpUniformBuffersMemory[i]);
        vkMapMemory(ctx.GetDevice(), mvpUniformBuffersMemory[i], 0, mvpBufferSize, 0, &mvpUniformBuffersMapped[i]);

        ctx.CreateBuffer(postProcessBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         postProcessUniformBuffers[i], postProcessUniformBuffersMemory[i]);
        vkMapMemory(ctx.GetDevice(), postProcessUniformBuffersMemory[i], 0, postProcessBufferSize, 0,
                    &postProcessUniformBuffersMapped[i]);
    }
}

void MotionBlurExample::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT * 5);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT * 15);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT * 5);
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(ctx.GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void MotionBlurExample::CreateDescriptorSets()
{
    VkDevice device = ctx.GetDevice();

    // G-Buffer descriptor sets
    {
        std::vector<VkDescriptorSetLayout> layouts(VulkanContext::MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutGBuffer);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsGBuffer.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsGBuffer.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate G-Buffer descriptor sets!");
        }

        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = mvpUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MotionBlurMVPUBO);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSetsGBuffer[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    // Motion Apply descriptor sets
    {
        std::vector<VkDescriptorSetLayout> layouts(VulkanContext::MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutPostProcess);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsMotionApply.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsMotionApply.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate motion apply descriptor sets!");
        }

        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::array<VkDescriptorImageInfo, 3> imageInfos{};
            imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[0].imageView = rtSceneColor.view;
            imageInfos[0].sampler = samplerLinear;

            imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[1].imageView = rtVelocity.view;
            imageInfos[1].sampler = samplerLinear;

            imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[2].imageView = rtDepth.view;
            imageInfos[2].sampler = samplerNearest;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = postProcessUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MotionBlurPostProcessParams);

            std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
            for (int j = 0; j < 3; j++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = descriptorSetsMotionApply[i];
                descriptorWrites[j].dstBinding = j;
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].descriptorCount = 1;
                descriptorWrites[j].pImageInfo = &imageInfos[j];
            }

            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = descriptorSetsMotionApply[i];
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[3].descriptorCount = 1;
            descriptorWrites[3].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                   nullptr);
        }
    }

    // Blur Vertical descriptor sets
    {
        std::vector<VkDescriptorSetLayout> layouts(VulkanContext::MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutPostProcess);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsBlurVertical.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsBlurVertical.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate blur vertical descriptor sets!");
        }

        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::array<VkDescriptorImageInfo, 3> imageInfos{};
            imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[0].imageView = rtMotion.view;
            imageInfos[0].sampler = samplerLinear;

            imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[1].imageView = rtVelocity.view;
            imageInfos[1].sampler = samplerLinear;

            imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[2].imageView = rtDepth.view;
            imageInfos[2].sampler = samplerNearest;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = postProcessUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MotionBlurPostProcessParams);

            std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
            for (int j = 0; j < 3; j++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = descriptorSetsBlurVertical[i];
                descriptorWrites[j].dstBinding = j;
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].descriptorCount = 1;
                descriptorWrites[j].pImageInfo = &imageInfos[j];
            }

            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = descriptorSetsBlurVertical[i];
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[3].descriptorCount = 1;
            descriptorWrites[3].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                   nullptr);
        }
    }

    // Blur Horizontal descriptor sets
    {
        std::vector<VkDescriptorSetLayout> layouts(VulkanContext::MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutPostProcess);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsBlurHorizontal.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsBlurHorizontal.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate blur horizontal descriptor sets!");
        }

        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::array<VkDescriptorImageInfo, 3> imageInfos{};
            imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[0].imageView = rtBlurIntermediate.view;
            imageInfos[0].sampler = samplerLinear;

            imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[1].imageView = rtVelocity.view;
            imageInfos[1].sampler = samplerLinear;

            imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[2].imageView = rtDepth.view;
            imageInfos[2].sampler = samplerNearest;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = postProcessUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MotionBlurPostProcessParams);

            std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
            for (int j = 0; j < 3; j++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = descriptorSetsBlurHorizontal[i];
                descriptorWrites[j].dstBinding = j;
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].descriptorCount = 1;
                descriptorWrites[j].pImageInfo = &imageInfos[j];
            }

            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = descriptorSetsBlurHorizontal[i];
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[3].descriptorCount = 1;
            descriptorWrites[3].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                   nullptr);
        }
    }

    // Final pass descriptor sets
    {
        std::vector<VkDescriptorSetLayout> layouts(VulkanContext::MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutFinal);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsFinal.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsFinal.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate final descriptor sets!");
        }

        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::array<VkDescriptorImageInfo, 2> imageInfos{};
            imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[0].imageView = rtMotion.view;
            imageInfos[0].sampler = samplerLinear;

            imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[1].imageView = rtBlurFinal.view;
            imageInfos[1].sampler = samplerLinear;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            for (int j = 0; j < 2; j++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = descriptorSetsFinal[i];
                descriptorWrites[j].dstBinding = j;
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].descriptorCount = 1;
                descriptorWrites[j].pImageInfo = &imageInfos[j];
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                   nullptr);
        }
    }
}

void MotionBlurExample::Update(float deltaTime)
{
    totalTime += deltaTime;

    uint32_t currentFrame = ctx.GetCurrentFrame();
    VkExtent2D extent = ctx.GetSwapChainExtent();

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), totalTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), extent.width / static_cast<float>(extent.height), 0.1f, 10.0f);
    proj[1][1] *= -1;

    glm::mat4 currentMVP = proj * view * model;

    MotionBlurMVPUBO ubo{};
    ubo.currMVP = currentMVP;
    ubo.prevMVP = previousMVP;

    memcpy(mvpUniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    previousMVP = currentMVP;

    MotionBlurPostProcessParams params{};
    params.blurStrength = 1.0f;
    params.motionScale = 1.0f;
    params.texelSize = glm::vec2(1.0f / extent.width, 1.0f / extent.height);

    memcpy(postProcessUniformBuffersMapped[currentFrame], &params, sizeof(params));
}

void MotionBlurExample::RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex)
{
    uint32_t currentFrame = ctx.GetCurrentFrame();
    VkExtent2D extent = ctx.GetSwapChainExtent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkClearValue clearVelocity = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
    VkClearValue clearDepth = {{{1.0f, 0}}};

    // Pass 0: G-Buffer (Triangle)
    {
        std::array<VkClearValue, 3> clearValues = {clearColor, clearVelocity, clearDepth};

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPassGBuffer;
        renderPassInfo.framebuffer = fbGBuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineGBuffer);

        VkBuffer vertexBuffers[] = {triangleVertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutGBuffer, 0, 1,
                                &descriptorSetsGBuffer[currentFrame], 0, nullptr);
        vkCmdDraw(cmd, static_cast<uint32_t>(triangleVertices.size()), 1, 0, 0);
        vkCmdEndRenderPass(cmd);
    }

    // Pass 1: Motion Apply
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPassMotionApply;
        renderPassInfo.framebuffer = fbMotionApply;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineMotionApply);

        fullscreenQuad.Bind(cmd);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutMotionApply, 0, 1,
                                &descriptorSetsMotionApply[currentFrame], 0, nullptr);
        fullscreenQuad.Draw(cmd);
        vkCmdEndRenderPass(cmd);
    }

    // Pass 2: Blur Vertical
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPassBlurVertical;
        renderPassInfo.framebuffer = fbBlurVertical;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBlurVertical);

        fullscreenQuad.Bind(cmd);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutBlur, 0, 1,
                                &descriptorSetsBlurVertical[currentFrame], 0, nullptr);
        fullscreenQuad.Draw(cmd);
        vkCmdEndRenderPass(cmd);
    }

    // Pass 3: Blur Horizontal
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPassBlurHorizontal;
        renderPassInfo.framebuffer = fbBlurHorizontal;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBlurHorizontal);

        fullscreenQuad.Bind(cmd);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutBlur, 0, 1,
                                &descriptorSetsBlurHorizontal[currentFrame], 0, nullptr);
        fullscreenQuad.Draw(cmd);
        vkCmdEndRenderPass(cmd);
    }

    // Pass 4: Final to Swapchain
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPassFinal;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineFinal);

        fullscreenQuad.Bind(cmd);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutFinal, 0, 1,
                                &descriptorSetsFinal[currentFrame], 0, nullptr);
        fullscreenQuad.Draw(cmd);
        vkCmdEndRenderPass(cmd);
    }
}

}  // namespace vkdemo
