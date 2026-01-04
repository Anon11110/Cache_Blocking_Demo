#pragma once

#include "../core/vulkan_context.h"

#include <string>

namespace vkdemo
{

class ExampleBase
{
public:
    explicit ExampleBase(VulkanContext& context) : ctx(context) {}
    virtual ~ExampleBase() = default;

    ExampleBase(const ExampleBase&) = delete;
    ExampleBase& operator=(const ExampleBase&) = delete;

    virtual std::string GetName() const = 0;

    virtual void Initialize() = 0;
    virtual void Cleanup() = 0;
    virtual void OnSwapChainRecreated() = 0;
    virtual void OnSwapChainCleanup() = 0;
    virtual void RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void ProcessInput(GLFWwindow* window, float deltaTime) {}

protected:
    VulkanContext& ctx;
};

}  // namespace vkdemo
