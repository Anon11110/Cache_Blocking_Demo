#include "core/vulkan_context.h"
#include "examples/example_base.h"
#include "examples/motion_blur/motion_blur_example.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{

std::unique_ptr<vkdemo::ExampleBase> CreateExample(vkdemo::VulkanContext& ctx)
{
    return std::make_unique<vkdemo::MotionBlurExample>(ctx);
}

}  // namespace

int main()
{
    try
    {
        vkdemo::VulkanContext context(1280, 720, "Cache Blocking Demo");

        auto example = CreateExample(context);

        std::cout << "Running: " << example->GetName() << std::endl;

        context.Run(example.get());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
