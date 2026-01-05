/**
 * volk
 *
 * Copyright (C) 2018-2023, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at https://github.com/zeux/volk
 *
 * This library is distributed under the MIT License. See notice at the end of this file.
 */
#define VK_NO_PROTOTYPES
#include "volk.h"

#ifdef _WIN32
    typedef const char* LPCSTR;
    typedef struct HINSTANCE__* HINSTANCE;
    typedef HINSTANCE HMODULE;
    #if defined(_MINWINDEF_)
        /* minwindef.h defines FARPROC, and attempting to redefine it may conflict with -Wstrict-prototypes */
    #elif defined(_WIN64)
        typedef __int64 (__stdcall* FARPROC)(void);
    #else
        typedef int (__stdcall* FARPROC)(void);
    #endif
    __declspec(dllimport) HMODULE __stdcall LoadLibraryA(LPCSTR);
    __declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE, LPCSTR);
    __declspec(dllimport) int __stdcall FreeLibrary(HMODULE);
#else
    #include <dlfcn.h>
#endif

static void volkGenLoadLoader(void* context, PFN_vkVoidFunction (*load)(void*, const char*));
static void volkGenLoadInstance(void* context, PFN_vkVoidFunction (*load)(void*, const char*));
static void volkGenLoadDevice(void* context, PFN_vkVoidFunction (*load)(void*, const char*));
static void volkGenLoadDeviceTable(struct VolkDeviceTable* table, void* context, PFN_vkVoidFunction (*load)(void*, const char*));

static PFN_vkVoidFunction vkGetInstanceProcAddrStub(void* context, const char* name)
{
    return vkGetInstanceProcAddr((VkInstance)context, name);
}

static PFN_vkVoidFunction vkGetDeviceProcAddrStub(void* context, const char* name)
{
    return vkGetDeviceProcAddr((VkDevice)context, name);
}

static PFN_vkVoidFunction nullProcAddrStub(void* context, const char* name)
{
    (void)context;
    (void)name;
    return 0;
}

#ifdef _WIN32
static HMODULE loadedModule = 0;
#else
static void* loadedModule = 0;
#endif

static VkInstance loadedInstance = VK_NULL_HANDLE;
static VkDevice loadedDevice = VK_NULL_HANDLE;

VkResult volkInitialize(void)
{
#if defined(_WIN32)
    HMODULE module = LoadLibraryA("vulkan-1.dll");
    if (!module)
        return VK_ERROR_INITIALIZATION_FAILED;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(void(*)(void))GetProcAddress(module, "vkGetInstanceProcAddr");
#elif defined(__APPLE__)
    void* module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        return VK_ERROR_INITIALIZATION_FAILED;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#else
    void* module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        return VK_ERROR_INITIALIZATION_FAILED;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#endif

    loadedModule = module;
    volkGenLoadLoader(0, nullProcAddrStub);
    volkGenLoadLoader(0, vkGetInstanceProcAddrStub);

    return VK_SUCCESS;
}

VkResult volkInitializeCustom(PFN_vkGetInstanceProcAddr handler)
{
    vkGetInstanceProcAddr = handler;

    volkGenLoadLoader(0, nullProcAddrStub);
    volkGenLoadLoader(0, vkGetInstanceProcAddrStub);

    return VK_SUCCESS;
}

uint32_t volkGetInstanceVersion(void)
{
#if defined(VK_VERSION_1_1)
    uint32_t apiVersion = 0;
    if (vkEnumerateInstanceVersion && vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS)
        return apiVersion;
#endif

    if (vkCreateInstance)
        return VK_API_VERSION_1_0;

    return 0;
}

void volkLoadInstance(VkInstance instance)
{
    loadedInstance = instance;
    volkGenLoadInstance(instance, vkGetInstanceProcAddrStub);
    volkGenLoadDevice(instance, vkGetInstanceProcAddrStub);
}

void volkLoadInstanceOnly(VkInstance instance)
{
    loadedInstance = instance;
    volkGenLoadInstance(instance, vkGetInstanceProcAddrStub);
}

VkInstance volkGetLoadedInstance(void)
{
    return loadedInstance;
}

void volkLoadDevice(VkDevice device)
{
    loadedDevice = device;
    volkGenLoadDevice(device, vkGetDeviceProcAddrStub);
}

VkDevice volkGetLoadedDevice(void)
{
    return loadedDevice;
}

void volkLoadDeviceTable(struct VolkDeviceTable* table, VkDevice device)
{
    volkGenLoadDeviceTable(table, device, vkGetDeviceProcAddrStub);
}

/* VOLK_GENERATE_PROTOTYPES_C */
#if defined(VK_VERSION_1_0)
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCmdClearColorImage vkCmdClearColorImage;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
PFN_vkCmdFillBuffer vkCmdFillBuffer;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdResetEvent vkCmdResetEvent;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdResolveImage vkCmdResolveImage;
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
PFN_vkCmdSetEvent vkCmdSetEvent;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
PFN_vkCmdWaitEvents vkCmdWaitEvents;
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkCreateBufferView vkCreateBufferView;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkCreateEvent vkCreateEvent;
PFN_vkCreateFence vkCreateFence;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateImage vkCreateImage;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkCreateInstance vkCreateInstance;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkDestroyBufferView vkDestroyBufferView;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkDestroyEvent vkDestroyEvent;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkGetEventStatus vkGetEventStatus;
PFN_vkGetFenceStatus vkGetFenceStatus;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkMapMemory vkMapMemory;
PFN_vkMergePipelineCaches vkMergePipelineCaches;
PFN_vkQueueBindSparse vkQueueBindSparse;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkResetCommandPool vkResetCommandPool;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkResetEvent vkResetEvent;
PFN_vkResetFences vkResetFences;
PFN_vkSetEvent vkSetEvent;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkWaitForFences vkWaitForFences;
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
PFN_vkBindBufferMemory2 vkBindBufferMemory2;
PFN_vkBindImageMemory2 vkBindImageMemory2;
PFN_vkCmdDispatchBase vkCmdDispatchBase;
PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask;
PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate;
PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion;
PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate;
PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion;
PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups;
PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport;
PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures;
PFN_vkGetDeviceQueue2 vkGetDeviceQueue2;
PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2;
PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties;
PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties;
PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2;
PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2;
PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2;
PFN_vkTrimCommandPool vkTrimCommandPool;
PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate;
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2;
PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2;
PFN_vkCmdNextSubpass2 vkCmdNextSubpass2;
PFN_vkCreateRenderPass2 vkCreateRenderPass2;
PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress;
PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress;
PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
PFN_vkResetQueryPool vkResetQueryPool;
PFN_vkSignalSemaphore vkSignalSemaphore;
PFN_vkWaitSemaphores vkWaitSemaphores;
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
PFN_vkCmdBeginRendering vkCmdBeginRendering;
PFN_vkCmdBindVertexBuffers2 vkCmdBindVertexBuffers2;
PFN_vkCmdBlitImage2 vkCmdBlitImage2;
PFN_vkCmdCopyBuffer2 vkCmdCopyBuffer2;
PFN_vkCmdCopyBufferToImage2 vkCmdCopyBufferToImage2;
PFN_vkCmdCopyImage2 vkCmdCopyImage2;
PFN_vkCmdCopyImageToBuffer2 vkCmdCopyImageToBuffer2;
PFN_vkCmdEndRendering vkCmdEndRendering;
PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2;
PFN_vkCmdResetEvent2 vkCmdResetEvent2;
PFN_vkCmdResolveImage2 vkCmdResolveImage2;
PFN_vkCmdSetCullMode vkCmdSetCullMode;
PFN_vkCmdSetDepthBiasEnable vkCmdSetDepthBiasEnable;
PFN_vkCmdSetDepthBoundsTestEnable vkCmdSetDepthBoundsTestEnable;
PFN_vkCmdSetDepthCompareOp vkCmdSetDepthCompareOp;
PFN_vkCmdSetDepthTestEnable vkCmdSetDepthTestEnable;
PFN_vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnable;
PFN_vkCmdSetEvent2 vkCmdSetEvent2;
PFN_vkCmdSetFrontFace vkCmdSetFrontFace;
PFN_vkCmdSetPrimitiveRestartEnable vkCmdSetPrimitiveRestartEnable;
PFN_vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopology;
PFN_vkCmdSetRasterizerDiscardEnable vkCmdSetRasterizerDiscardEnable;
PFN_vkCmdSetScissorWithCount vkCmdSetScissorWithCount;
PFN_vkCmdSetStencilOp vkCmdSetStencilOp;
PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnable;
PFN_vkCmdSetViewportWithCount vkCmdSetViewportWithCount;
PFN_vkCmdWaitEvents2 vkCmdWaitEvents2;
PFN_vkCmdWriteTimestamp2 vkCmdWriteTimestamp2;
PFN_vkCreatePrivateDataSlot vkCreatePrivateDataSlot;
PFN_vkDestroyPrivateDataSlot vkDestroyPrivateDataSlot;
PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements;
PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements;
PFN_vkGetDeviceImageSparseMemoryRequirements vkGetDeviceImageSparseMemoryRequirements;
PFN_vkGetPhysicalDeviceToolProperties vkGetPhysicalDeviceToolProperties;
PFN_vkGetPrivateData vkGetPrivateData;
PFN_vkQueueSubmit2 vkQueueSubmit2;
PFN_vkSetPrivateData vkSetPrivateData;
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_debug_utils)
PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT;
PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTagEXT;
PFN_vkSubmitDebugUtilsMessageEXT vkSubmitDebugUtilsMessageEXT;
#endif /* defined(VK_EXT_debug_utils) */
#if defined(VK_KHR_surface)
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
#endif /* defined(VK_KHR_surface) */
#if defined(VK_KHR_swapchain)
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;
#endif /* defined(VK_KHR_swapchain) */
#if defined(VK_KHR_win32_surface)
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif /* defined(VK_KHR_win32_surface) */
#if defined(VK_KHR_xcb_surface)
PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;
PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif /* defined(VK_KHR_xcb_surface) */
#if defined(VK_KHR_xlib_surface)
PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif /* defined(VK_KHR_xlib_surface) */
#if defined(VK_KHR_wayland_surface)
PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif /* defined(VK_KHR_wayland_surface) */

static void volkGenLoadLoader(void* context, PFN_vkVoidFunction (*load)(void*, const char*))
{
    /* VOLK_GENERATE_LOAD_LOADER */
#if defined(VK_VERSION_1_0)
    vkCreateInstance = (PFN_vkCreateInstance)load(context, "vkCreateInstance");
    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)load(context, "vkEnumerateInstanceExtensionProperties");
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)load(context, "vkEnumerateInstanceLayerProperties");
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
    vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)load(context, "vkEnumerateInstanceVersion");
#endif /* defined(VK_VERSION_1_1) */
}

static void volkGenLoadInstance(void* context, PFN_vkVoidFunction (*load)(void*, const char*))
{
    /* VOLK_GENERATE_LOAD_INSTANCE */
#if defined(VK_VERSION_1_0)
    vkCreateDevice = (PFN_vkCreateDevice)load(context, "vkCreateDevice");
    vkDestroyInstance = (PFN_vkDestroyInstance)load(context, "vkDestroyInstance");
    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)load(context, "vkEnumerateDeviceExtensionProperties");
    vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)load(context, "vkEnumerateDeviceLayerProperties");
    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)load(context, "vkEnumeratePhysicalDevices");
    vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)load(context, "vkGetDeviceProcAddr");
    vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)load(context, "vkGetPhysicalDeviceFeatures");
    vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)load(context, "vkGetPhysicalDeviceFormatProperties");
    vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)load(context, "vkGetPhysicalDeviceImageFormatProperties");
    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)load(context, "vkGetPhysicalDeviceMemoryProperties");
    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)load(context, "vkGetPhysicalDeviceProperties");
    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)load(context, "vkGetPhysicalDeviceQueueFamilyProperties");
    vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)load(context, "vkGetPhysicalDeviceSparseImageFormatProperties");
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
    vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)load(context, "vkEnumeratePhysicalDeviceGroups");
    vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties)load(context, "vkGetPhysicalDeviceExternalBufferProperties");
    vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties)load(context, "vkGetPhysicalDeviceExternalFenceProperties");
    vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)load(context, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)load(context, "vkGetPhysicalDeviceFeatures2");
    vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2)load(context, "vkGetPhysicalDeviceFormatProperties2");
    vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)load(context, "vkGetPhysicalDeviceImageFormatProperties2");
    vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)load(context, "vkGetPhysicalDeviceMemoryProperties2");
    vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)load(context, "vkGetPhysicalDeviceProperties2");
    vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)load(context, "vkGetPhysicalDeviceQueueFamilyProperties2");
    vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)load(context, "vkGetPhysicalDeviceSparseImageFormatProperties2");
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_3)
    vkGetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties)load(context, "vkGetPhysicalDeviceToolProperties");
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_debug_utils)
    vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)load(context, "vkCmdBeginDebugUtilsLabelEXT");
    vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)load(context, "vkCmdEndDebugUtilsLabelEXT");
    vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)load(context, "vkCmdInsertDebugUtilsLabelEXT");
    vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)load(context, "vkCreateDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)load(context, "vkDestroyDebugUtilsMessengerEXT");
    vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)load(context, "vkQueueBeginDebugUtilsLabelEXT");
    vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)load(context, "vkQueueEndDebugUtilsLabelEXT");
    vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)load(context, "vkQueueInsertDebugUtilsLabelEXT");
    vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)load(context, "vkSetDebugUtilsObjectNameEXT");
    vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)load(context, "vkSetDebugUtilsObjectTagEXT");
    vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)load(context, "vkSubmitDebugUtilsMessageEXT");
#endif /* defined(VK_EXT_debug_utils) */
#if defined(VK_KHR_surface)
    vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)load(context, "vkDestroySurfaceKHR");
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)load(context, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)load(context, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)load(context, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)load(context, "vkGetPhysicalDeviceSurfaceSupportKHR");
#endif /* defined(VK_KHR_surface) */
#if defined(VK_KHR_win32_surface)
    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)load(context, "vkCreateWin32SurfaceKHR");
    vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)load(context, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* defined(VK_KHR_win32_surface) */
#if defined(VK_KHR_xcb_surface)
    vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)load(context, "vkCreateXcbSurfaceKHR");
    vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)load(context, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* defined(VK_KHR_xcb_surface) */
#if defined(VK_KHR_xlib_surface)
    vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)load(context, "vkCreateXlibSurfaceKHR");
    vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)load(context, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* defined(VK_KHR_xlib_surface) */
#if defined(VK_KHR_wayland_surface)
    vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)load(context, "vkCreateWaylandSurfaceKHR");
    vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)load(context, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* defined(VK_KHR_wayland_surface) */
}

static void volkGenLoadDevice(void* context, PFN_vkVoidFunction (*load)(void*, const char*))
{
    /* VOLK_GENERATE_LOAD_DEVICE */
#if defined(VK_VERSION_1_0)
    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)load(context, "vkAllocateCommandBuffers");
    vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)load(context, "vkAllocateDescriptorSets");
    vkAllocateMemory = (PFN_vkAllocateMemory)load(context, "vkAllocateMemory");
    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)load(context, "vkBeginCommandBuffer");
    vkBindBufferMemory = (PFN_vkBindBufferMemory)load(context, "vkBindBufferMemory");
    vkBindImageMemory = (PFN_vkBindImageMemory)load(context, "vkBindImageMemory");
    vkCmdBeginQuery = (PFN_vkCmdBeginQuery)load(context, "vkCmdBeginQuery");
    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)load(context, "vkCmdBeginRenderPass");
    vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)load(context, "vkCmdBindDescriptorSets");
    vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)load(context, "vkCmdBindIndexBuffer");
    vkCmdBindPipeline = (PFN_vkCmdBindPipeline)load(context, "vkCmdBindPipeline");
    vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)load(context, "vkCmdBindVertexBuffers");
    vkCmdBlitImage = (PFN_vkCmdBlitImage)load(context, "vkCmdBlitImage");
    vkCmdClearAttachments = (PFN_vkCmdClearAttachments)load(context, "vkCmdClearAttachments");
    vkCmdClearColorImage = (PFN_vkCmdClearColorImage)load(context, "vkCmdClearColorImage");
    vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)load(context, "vkCmdClearDepthStencilImage");
    vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)load(context, "vkCmdCopyBuffer");
    vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)load(context, "vkCmdCopyBufferToImage");
    vkCmdCopyImage = (PFN_vkCmdCopyImage)load(context, "vkCmdCopyImage");
    vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)load(context, "vkCmdCopyImageToBuffer");
    vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)load(context, "vkCmdCopyQueryPoolResults");
    vkCmdDispatch = (PFN_vkCmdDispatch)load(context, "vkCmdDispatch");
    vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)load(context, "vkCmdDispatchIndirect");
    vkCmdDraw = (PFN_vkCmdDraw)load(context, "vkCmdDraw");
    vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)load(context, "vkCmdDrawIndexed");
    vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)load(context, "vkCmdDrawIndexedIndirect");
    vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)load(context, "vkCmdDrawIndirect");
    vkCmdEndQuery = (PFN_vkCmdEndQuery)load(context, "vkCmdEndQuery");
    vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)load(context, "vkCmdEndRenderPass");
    vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)load(context, "vkCmdExecuteCommands");
    vkCmdFillBuffer = (PFN_vkCmdFillBuffer)load(context, "vkCmdFillBuffer");
    vkCmdNextSubpass = (PFN_vkCmdNextSubpass)load(context, "vkCmdNextSubpass");
    vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)load(context, "vkCmdPipelineBarrier");
    vkCmdPushConstants = (PFN_vkCmdPushConstants)load(context, "vkCmdPushConstants");
    vkCmdResetEvent = (PFN_vkCmdResetEvent)load(context, "vkCmdResetEvent");
    vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)load(context, "vkCmdResetQueryPool");
    vkCmdResolveImage = (PFN_vkCmdResolveImage)load(context, "vkCmdResolveImage");
    vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)load(context, "vkCmdSetBlendConstants");
    vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)load(context, "vkCmdSetDepthBias");
    vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)load(context, "vkCmdSetDepthBounds");
    vkCmdSetEvent = (PFN_vkCmdSetEvent)load(context, "vkCmdSetEvent");
    vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)load(context, "vkCmdSetLineWidth");
    vkCmdSetScissor = (PFN_vkCmdSetScissor)load(context, "vkCmdSetScissor");
    vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)load(context, "vkCmdSetStencilCompareMask");
    vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference)load(context, "vkCmdSetStencilReference");
    vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)load(context, "vkCmdSetStencilWriteMask");
    vkCmdSetViewport = (PFN_vkCmdSetViewport)load(context, "vkCmdSetViewport");
    vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)load(context, "vkCmdUpdateBuffer");
    vkCmdWaitEvents = (PFN_vkCmdWaitEvents)load(context, "vkCmdWaitEvents");
    vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)load(context, "vkCmdWriteTimestamp");
    vkCreateBuffer = (PFN_vkCreateBuffer)load(context, "vkCreateBuffer");
    vkCreateBufferView = (PFN_vkCreateBufferView)load(context, "vkCreateBufferView");
    vkCreateCommandPool = (PFN_vkCreateCommandPool)load(context, "vkCreateCommandPool");
    vkCreateComputePipelines = (PFN_vkCreateComputePipelines)load(context, "vkCreateComputePipelines");
    vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)load(context, "vkCreateDescriptorPool");
    vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)load(context, "vkCreateDescriptorSetLayout");
    vkCreateEvent = (PFN_vkCreateEvent)load(context, "vkCreateEvent");
    vkCreateFence = (PFN_vkCreateFence)load(context, "vkCreateFence");
    vkCreateFramebuffer = (PFN_vkCreateFramebuffer)load(context, "vkCreateFramebuffer");
    vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)load(context, "vkCreateGraphicsPipelines");
    vkCreateImage = (PFN_vkCreateImage)load(context, "vkCreateImage");
    vkCreateImageView = (PFN_vkCreateImageView)load(context, "vkCreateImageView");
    vkCreatePipelineCache = (PFN_vkCreatePipelineCache)load(context, "vkCreatePipelineCache");
    vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)load(context, "vkCreatePipelineLayout");
    vkCreateQueryPool = (PFN_vkCreateQueryPool)load(context, "vkCreateQueryPool");
    vkCreateRenderPass = (PFN_vkCreateRenderPass)load(context, "vkCreateRenderPass");
    vkCreateSampler = (PFN_vkCreateSampler)load(context, "vkCreateSampler");
    vkCreateSemaphore = (PFN_vkCreateSemaphore)load(context, "vkCreateSemaphore");
    vkCreateShaderModule = (PFN_vkCreateShaderModule)load(context, "vkCreateShaderModule");
    vkDestroyBuffer = (PFN_vkDestroyBuffer)load(context, "vkDestroyBuffer");
    vkDestroyBufferView = (PFN_vkDestroyBufferView)load(context, "vkDestroyBufferView");
    vkDestroyCommandPool = (PFN_vkDestroyCommandPool)load(context, "vkDestroyCommandPool");
    vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)load(context, "vkDestroyDescriptorPool");
    vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)load(context, "vkDestroyDescriptorSetLayout");
    vkDestroyDevice = (PFN_vkDestroyDevice)load(context, "vkDestroyDevice");
    vkDestroyEvent = (PFN_vkDestroyEvent)load(context, "vkDestroyEvent");
    vkDestroyFence = (PFN_vkDestroyFence)load(context, "vkDestroyFence");
    vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)load(context, "vkDestroyFramebuffer");
    vkDestroyImage = (PFN_vkDestroyImage)load(context, "vkDestroyImage");
    vkDestroyImageView = (PFN_vkDestroyImageView)load(context, "vkDestroyImageView");
    vkDestroyPipeline = (PFN_vkDestroyPipeline)load(context, "vkDestroyPipeline");
    vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache)load(context, "vkDestroyPipelineCache");
    vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)load(context, "vkDestroyPipelineLayout");
    vkDestroyQueryPool = (PFN_vkDestroyQueryPool)load(context, "vkDestroyQueryPool");
    vkDestroyRenderPass = (PFN_vkDestroyRenderPass)load(context, "vkDestroyRenderPass");
    vkDestroySampler = (PFN_vkDestroySampler)load(context, "vkDestroySampler");
    vkDestroySemaphore = (PFN_vkDestroySemaphore)load(context, "vkDestroySemaphore");
    vkDestroyShaderModule = (PFN_vkDestroyShaderModule)load(context, "vkDestroyShaderModule");
    vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)load(context, "vkDeviceWaitIdle");
    vkEndCommandBuffer = (PFN_vkEndCommandBuffer)load(context, "vkEndCommandBuffer");
    vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)load(context, "vkFlushMappedMemoryRanges");
    vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)load(context, "vkFreeCommandBuffers");
    vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets)load(context, "vkFreeDescriptorSets");
    vkFreeMemory = (PFN_vkFreeMemory)load(context, "vkFreeMemory");
    vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)load(context, "vkGetBufferMemoryRequirements");
    vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)load(context, "vkGetDeviceMemoryCommitment");
    vkGetDeviceQueue = (PFN_vkGetDeviceQueue)load(context, "vkGetDeviceQueue");
    vkGetEventStatus = (PFN_vkGetEventStatus)load(context, "vkGetEventStatus");
    vkGetFenceStatus = (PFN_vkGetFenceStatus)load(context, "vkGetFenceStatus");
    vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)load(context, "vkGetImageMemoryRequirements");
    vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)load(context, "vkGetImageSparseMemoryRequirements");
    vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)load(context, "vkGetImageSubresourceLayout");
    vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData)load(context, "vkGetPipelineCacheData");
    vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)load(context, "vkGetQueryPoolResults");
    vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)load(context, "vkGetRenderAreaGranularity");
    vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)load(context, "vkInvalidateMappedMemoryRanges");
    vkMapMemory = (PFN_vkMapMemory)load(context, "vkMapMemory");
    vkMergePipelineCaches = (PFN_vkMergePipelineCaches)load(context, "vkMergePipelineCaches");
    vkQueueBindSparse = (PFN_vkQueueBindSparse)load(context, "vkQueueBindSparse");
    vkQueueSubmit = (PFN_vkQueueSubmit)load(context, "vkQueueSubmit");
    vkQueueWaitIdle = (PFN_vkQueueWaitIdle)load(context, "vkQueueWaitIdle");
    vkResetCommandBuffer = (PFN_vkResetCommandBuffer)load(context, "vkResetCommandBuffer");
    vkResetCommandPool = (PFN_vkResetCommandPool)load(context, "vkResetCommandPool");
    vkResetDescriptorPool = (PFN_vkResetDescriptorPool)load(context, "vkResetDescriptorPool");
    vkResetEvent = (PFN_vkResetEvent)load(context, "vkResetEvent");
    vkResetFences = (PFN_vkResetFences)load(context, "vkResetFences");
    vkSetEvent = (PFN_vkSetEvent)load(context, "vkSetEvent");
    vkUnmapMemory = (PFN_vkUnmapMemory)load(context, "vkUnmapMemory");
    vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)load(context, "vkUpdateDescriptorSets");
    vkWaitForFences = (PFN_vkWaitForFences)load(context, "vkWaitForFences");
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
    vkBindBufferMemory2 = (PFN_vkBindBufferMemory2)load(context, "vkBindBufferMemory2");
    vkBindImageMemory2 = (PFN_vkBindImageMemory2)load(context, "vkBindImageMemory2");
    vkCmdDispatchBase = (PFN_vkCmdDispatchBase)load(context, "vkCmdDispatchBase");
    vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)load(context, "vkCmdSetDeviceMask");
    vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)load(context, "vkCreateDescriptorUpdateTemplate");
    vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)load(context, "vkCreateSamplerYcbcrConversion");
    vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)load(context, "vkDestroyDescriptorUpdateTemplate");
    vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)load(context, "vkDestroySamplerYcbcrConversion");
    vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)load(context, "vkGetBufferMemoryRequirements2");
    vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)load(context, "vkGetDescriptorSetLayoutSupport");
    vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)load(context, "vkGetDeviceGroupPeerMemoryFeatures");
    vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)load(context, "vkGetDeviceQueue2");
    vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)load(context, "vkGetImageMemoryRequirements2");
    vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)load(context, "vkGetImageSparseMemoryRequirements2");
    vkTrimCommandPool = (PFN_vkTrimCommandPool)load(context, "vkTrimCommandPool");
    vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)load(context, "vkUpdateDescriptorSetWithTemplate");
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
    vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)load(context, "vkCmdBeginRenderPass2");
    vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)load(context, "vkCmdDrawIndexedIndirectCount");
    vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)load(context, "vkCmdDrawIndirectCount");
    vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)load(context, "vkCmdEndRenderPass2");
    vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2)load(context, "vkCmdNextSubpass2");
    vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)load(context, "vkCreateRenderPass2");
    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)load(context, "vkGetBufferDeviceAddress");
    vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)load(context, "vkGetBufferOpaqueCaptureAddress");
    vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)load(context, "vkGetDeviceMemoryOpaqueCaptureAddress");
    vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)load(context, "vkGetSemaphoreCounterValue");
    vkResetQueryPool = (PFN_vkResetQueryPool)load(context, "vkResetQueryPool");
    vkSignalSemaphore = (PFN_vkSignalSemaphore)load(context, "vkSignalSemaphore");
    vkWaitSemaphores = (PFN_vkWaitSemaphores)load(context, "vkWaitSemaphores");
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
    vkCmdBeginRendering = (PFN_vkCmdBeginRendering)load(context, "vkCmdBeginRendering");
    vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)load(context, "vkCmdBindVertexBuffers2");
    vkCmdBlitImage2 = (PFN_vkCmdBlitImage2)load(context, "vkCmdBlitImage2");
    vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)load(context, "vkCmdCopyBuffer2");
    vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)load(context, "vkCmdCopyBufferToImage2");
    vkCmdCopyImage2 = (PFN_vkCmdCopyImage2)load(context, "vkCmdCopyImage2");
    vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)load(context, "vkCmdCopyImageToBuffer2");
    vkCmdEndRendering = (PFN_vkCmdEndRendering)load(context, "vkCmdEndRendering");
    vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)load(context, "vkCmdPipelineBarrier2");
    vkCmdResetEvent2 = (PFN_vkCmdResetEvent2)load(context, "vkCmdResetEvent2");
    vkCmdResolveImage2 = (PFN_vkCmdResolveImage2)load(context, "vkCmdResolveImage2");
    vkCmdSetCullMode = (PFN_vkCmdSetCullMode)load(context, "vkCmdSetCullMode");
    vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)load(context, "vkCmdSetDepthBiasEnable");
    vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)load(context, "vkCmdSetDepthBoundsTestEnable");
    vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)load(context, "vkCmdSetDepthCompareOp");
    vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)load(context, "vkCmdSetDepthTestEnable");
    vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)load(context, "vkCmdSetDepthWriteEnable");
    vkCmdSetEvent2 = (PFN_vkCmdSetEvent2)load(context, "vkCmdSetEvent2");
    vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace)load(context, "vkCmdSetFrontFace");
    vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)load(context, "vkCmdSetPrimitiveRestartEnable");
    vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)load(context, "vkCmdSetPrimitiveTopology");
    vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)load(context, "vkCmdSetRasterizerDiscardEnable");
    vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)load(context, "vkCmdSetScissorWithCount");
    vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp)load(context, "vkCmdSetStencilOp");
    vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)load(context, "vkCmdSetStencilTestEnable");
    vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)load(context, "vkCmdSetViewportWithCount");
    vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2)load(context, "vkCmdWaitEvents2");
    vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)load(context, "vkCmdWriteTimestamp2");
    vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)load(context, "vkCreatePrivateDataSlot");
    vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)load(context, "vkDestroyPrivateDataSlot");
    vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)load(context, "vkGetDeviceBufferMemoryRequirements");
    vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)load(context, "vkGetDeviceImageMemoryRequirements");
    vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements)load(context, "vkGetDeviceImageSparseMemoryRequirements");
    vkGetPrivateData = (PFN_vkGetPrivateData)load(context, "vkGetPrivateData");
    vkQueueSubmit2 = (PFN_vkQueueSubmit2)load(context, "vkQueueSubmit2");
    vkSetPrivateData = (PFN_vkSetPrivateData)load(context, "vkSetPrivateData");
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_KHR_swapchain)
    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)load(context, "vkAcquireNextImageKHR");
    vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)load(context, "vkCreateSwapchainKHR");
    vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)load(context, "vkDestroySwapchainKHR");
    vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)load(context, "vkGetSwapchainImagesKHR");
    vkQueuePresentKHR = (PFN_vkQueuePresentKHR)load(context, "vkQueuePresentKHR");
#endif /* defined(VK_KHR_swapchain) */
}

static void volkGenLoadDeviceTable(struct VolkDeviceTable* table, void* context, PFN_vkVoidFunction (*load)(void*, const char*))
{
    /* VOLK_GENERATE_LOAD_DEVICE_TABLE */
#if defined(VK_VERSION_1_0)
    table->vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)load(context, "vkAllocateCommandBuffers");
    table->vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)load(context, "vkAllocateDescriptorSets");
    table->vkAllocateMemory = (PFN_vkAllocateMemory)load(context, "vkAllocateMemory");
    table->vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)load(context, "vkBeginCommandBuffer");
    table->vkBindBufferMemory = (PFN_vkBindBufferMemory)load(context, "vkBindBufferMemory");
    table->vkBindImageMemory = (PFN_vkBindImageMemory)load(context, "vkBindImageMemory");
    table->vkCmdBeginQuery = (PFN_vkCmdBeginQuery)load(context, "vkCmdBeginQuery");
    table->vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)load(context, "vkCmdBeginRenderPass");
    table->vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)load(context, "vkCmdBindDescriptorSets");
    table->vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)load(context, "vkCmdBindIndexBuffer");
    table->vkCmdBindPipeline = (PFN_vkCmdBindPipeline)load(context, "vkCmdBindPipeline");
    table->vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)load(context, "vkCmdBindVertexBuffers");
    table->vkCmdBlitImage = (PFN_vkCmdBlitImage)load(context, "vkCmdBlitImage");
    table->vkCmdClearAttachments = (PFN_vkCmdClearAttachments)load(context, "vkCmdClearAttachments");
    table->vkCmdClearColorImage = (PFN_vkCmdClearColorImage)load(context, "vkCmdClearColorImage");
    table->vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)load(context, "vkCmdClearDepthStencilImage");
    table->vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)load(context, "vkCmdCopyBuffer");
    table->vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)load(context, "vkCmdCopyBufferToImage");
    table->vkCmdCopyImage = (PFN_vkCmdCopyImage)load(context, "vkCmdCopyImage");
    table->vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)load(context, "vkCmdCopyImageToBuffer");
    table->vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)load(context, "vkCmdCopyQueryPoolResults");
    table->vkCmdDispatch = (PFN_vkCmdDispatch)load(context, "vkCmdDispatch");
    table->vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)load(context, "vkCmdDispatchIndirect");
    table->vkCmdDraw = (PFN_vkCmdDraw)load(context, "vkCmdDraw");
    table->vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)load(context, "vkCmdDrawIndexed");
    table->vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)load(context, "vkCmdDrawIndexedIndirect");
    table->vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)load(context, "vkCmdDrawIndirect");
    table->vkCmdEndQuery = (PFN_vkCmdEndQuery)load(context, "vkCmdEndQuery");
    table->vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)load(context, "vkCmdEndRenderPass");
    table->vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)load(context, "vkCmdExecuteCommands");
    table->vkCmdFillBuffer = (PFN_vkCmdFillBuffer)load(context, "vkCmdFillBuffer");
    table->vkCmdNextSubpass = (PFN_vkCmdNextSubpass)load(context, "vkCmdNextSubpass");
    table->vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)load(context, "vkCmdPipelineBarrier");
    table->vkCmdPushConstants = (PFN_vkCmdPushConstants)load(context, "vkCmdPushConstants");
    table->vkCmdResetEvent = (PFN_vkCmdResetEvent)load(context, "vkCmdResetEvent");
    table->vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)load(context, "vkCmdResetQueryPool");
    table->vkCmdResolveImage = (PFN_vkCmdResolveImage)load(context, "vkCmdResolveImage");
    table->vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)load(context, "vkCmdSetBlendConstants");
    table->vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)load(context, "vkCmdSetDepthBias");
    table->vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)load(context, "vkCmdSetDepthBounds");
    table->vkCmdSetEvent = (PFN_vkCmdSetEvent)load(context, "vkCmdSetEvent");
    table->vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)load(context, "vkCmdSetLineWidth");
    table->vkCmdSetScissor = (PFN_vkCmdSetScissor)load(context, "vkCmdSetScissor");
    table->vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)load(context, "vkCmdSetStencilCompareMask");
    table->vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference)load(context, "vkCmdSetStencilReference");
    table->vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)load(context, "vkCmdSetStencilWriteMask");
    table->vkCmdSetViewport = (PFN_vkCmdSetViewport)load(context, "vkCmdSetViewport");
    table->vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)load(context, "vkCmdUpdateBuffer");
    table->vkCmdWaitEvents = (PFN_vkCmdWaitEvents)load(context, "vkCmdWaitEvents");
    table->vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)load(context, "vkCmdWriteTimestamp");
    table->vkCreateBuffer = (PFN_vkCreateBuffer)load(context, "vkCreateBuffer");
    table->vkCreateBufferView = (PFN_vkCreateBufferView)load(context, "vkCreateBufferView");
    table->vkCreateCommandPool = (PFN_vkCreateCommandPool)load(context, "vkCreateCommandPool");
    table->vkCreateComputePipelines = (PFN_vkCreateComputePipelines)load(context, "vkCreateComputePipelines");
    table->vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)load(context, "vkCreateDescriptorPool");
    table->vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)load(context, "vkCreateDescriptorSetLayout");
    table->vkCreateEvent = (PFN_vkCreateEvent)load(context, "vkCreateEvent");
    table->vkCreateFence = (PFN_vkCreateFence)load(context, "vkCreateFence");
    table->vkCreateFramebuffer = (PFN_vkCreateFramebuffer)load(context, "vkCreateFramebuffer");
    table->vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)load(context, "vkCreateGraphicsPipelines");
    table->vkCreateImage = (PFN_vkCreateImage)load(context, "vkCreateImage");
    table->vkCreateImageView = (PFN_vkCreateImageView)load(context, "vkCreateImageView");
    table->vkCreatePipelineCache = (PFN_vkCreatePipelineCache)load(context, "vkCreatePipelineCache");
    table->vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)load(context, "vkCreatePipelineLayout");
    table->vkCreateQueryPool = (PFN_vkCreateQueryPool)load(context, "vkCreateQueryPool");
    table->vkCreateRenderPass = (PFN_vkCreateRenderPass)load(context, "vkCreateRenderPass");
    table->vkCreateSampler = (PFN_vkCreateSampler)load(context, "vkCreateSampler");
    table->vkCreateSemaphore = (PFN_vkCreateSemaphore)load(context, "vkCreateSemaphore");
    table->vkCreateShaderModule = (PFN_vkCreateShaderModule)load(context, "vkCreateShaderModule");
    table->vkDestroyBuffer = (PFN_vkDestroyBuffer)load(context, "vkDestroyBuffer");
    table->vkDestroyBufferView = (PFN_vkDestroyBufferView)load(context, "vkDestroyBufferView");
    table->vkDestroyCommandPool = (PFN_vkDestroyCommandPool)load(context, "vkDestroyCommandPool");
    table->vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)load(context, "vkDestroyDescriptorPool");
    table->vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)load(context, "vkDestroyDescriptorSetLayout");
    table->vkDestroyDevice = (PFN_vkDestroyDevice)load(context, "vkDestroyDevice");
    table->vkDestroyEvent = (PFN_vkDestroyEvent)load(context, "vkDestroyEvent");
    table->vkDestroyFence = (PFN_vkDestroyFence)load(context, "vkDestroyFence");
    table->vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)load(context, "vkDestroyFramebuffer");
    table->vkDestroyImage = (PFN_vkDestroyImage)load(context, "vkDestroyImage");
    table->vkDestroyImageView = (PFN_vkDestroyImageView)load(context, "vkDestroyImageView");
    table->vkDestroyPipeline = (PFN_vkDestroyPipeline)load(context, "vkDestroyPipeline");
    table->vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache)load(context, "vkDestroyPipelineCache");
    table->vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)load(context, "vkDestroyPipelineLayout");
    table->vkDestroyQueryPool = (PFN_vkDestroyQueryPool)load(context, "vkDestroyQueryPool");
    table->vkDestroyRenderPass = (PFN_vkDestroyRenderPass)load(context, "vkDestroyRenderPass");
    table->vkDestroySampler = (PFN_vkDestroySampler)load(context, "vkDestroySampler");
    table->vkDestroySemaphore = (PFN_vkDestroySemaphore)load(context, "vkDestroySemaphore");
    table->vkDestroyShaderModule = (PFN_vkDestroyShaderModule)load(context, "vkDestroyShaderModule");
    table->vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)load(context, "vkDeviceWaitIdle");
    table->vkEndCommandBuffer = (PFN_vkEndCommandBuffer)load(context, "vkEndCommandBuffer");
    table->vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)load(context, "vkFlushMappedMemoryRanges");
    table->vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)load(context, "vkFreeCommandBuffers");
    table->vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets)load(context, "vkFreeDescriptorSets");
    table->vkFreeMemory = (PFN_vkFreeMemory)load(context, "vkFreeMemory");
    table->vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)load(context, "vkGetBufferMemoryRequirements");
    table->vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)load(context, "vkGetDeviceMemoryCommitment");
    table->vkGetDeviceQueue = (PFN_vkGetDeviceQueue)load(context, "vkGetDeviceQueue");
    table->vkGetEventStatus = (PFN_vkGetEventStatus)load(context, "vkGetEventStatus");
    table->vkGetFenceStatus = (PFN_vkGetFenceStatus)load(context, "vkGetFenceStatus");
    table->vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)load(context, "vkGetImageMemoryRequirements");
    table->vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)load(context, "vkGetImageSparseMemoryRequirements");
    table->vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)load(context, "vkGetImageSubresourceLayout");
    table->vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData)load(context, "vkGetPipelineCacheData");
    table->vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)load(context, "vkGetQueryPoolResults");
    table->vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)load(context, "vkGetRenderAreaGranularity");
    table->vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)load(context, "vkInvalidateMappedMemoryRanges");
    table->vkMapMemory = (PFN_vkMapMemory)load(context, "vkMapMemory");
    table->vkMergePipelineCaches = (PFN_vkMergePipelineCaches)load(context, "vkMergePipelineCaches");
    table->vkQueueBindSparse = (PFN_vkQueueBindSparse)load(context, "vkQueueBindSparse");
    table->vkQueueSubmit = (PFN_vkQueueSubmit)load(context, "vkQueueSubmit");
    table->vkQueueWaitIdle = (PFN_vkQueueWaitIdle)load(context, "vkQueueWaitIdle");
    table->vkResetCommandBuffer = (PFN_vkResetCommandBuffer)load(context, "vkResetCommandBuffer");
    table->vkResetCommandPool = (PFN_vkResetCommandPool)load(context, "vkResetCommandPool");
    table->vkResetDescriptorPool = (PFN_vkResetDescriptorPool)load(context, "vkResetDescriptorPool");
    table->vkResetEvent = (PFN_vkResetEvent)load(context, "vkResetEvent");
    table->vkResetFences = (PFN_vkResetFences)load(context, "vkResetFences");
    table->vkSetEvent = (PFN_vkSetEvent)load(context, "vkSetEvent");
    table->vkUnmapMemory = (PFN_vkUnmapMemory)load(context, "vkUnmapMemory");
    table->vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)load(context, "vkUpdateDescriptorSets");
    table->vkWaitForFences = (PFN_vkWaitForFences)load(context, "vkWaitForFences");
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
    table->vkBindBufferMemory2 = (PFN_vkBindBufferMemory2)load(context, "vkBindBufferMemory2");
    table->vkBindImageMemory2 = (PFN_vkBindImageMemory2)load(context, "vkBindImageMemory2");
    table->vkCmdDispatchBase = (PFN_vkCmdDispatchBase)load(context, "vkCmdDispatchBase");
    table->vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)load(context, "vkCmdSetDeviceMask");
    table->vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)load(context, "vkCreateDescriptorUpdateTemplate");
    table->vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)load(context, "vkCreateSamplerYcbcrConversion");
    table->vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)load(context, "vkDestroyDescriptorUpdateTemplate");
    table->vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)load(context, "vkDestroySamplerYcbcrConversion");
    table->vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)load(context, "vkGetBufferMemoryRequirements2");
    table->vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)load(context, "vkGetDescriptorSetLayoutSupport");
    table->vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)load(context, "vkGetDeviceGroupPeerMemoryFeatures");
    table->vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)load(context, "vkGetDeviceQueue2");
    table->vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)load(context, "vkGetImageMemoryRequirements2");
    table->vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)load(context, "vkGetImageSparseMemoryRequirements2");
    table->vkTrimCommandPool = (PFN_vkTrimCommandPool)load(context, "vkTrimCommandPool");
    table->vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)load(context, "vkUpdateDescriptorSetWithTemplate");
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
    table->vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)load(context, "vkCmdBeginRenderPass2");
    table->vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)load(context, "vkCmdDrawIndexedIndirectCount");
    table->vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)load(context, "vkCmdDrawIndirectCount");
    table->vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)load(context, "vkCmdEndRenderPass2");
    table->vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2)load(context, "vkCmdNextSubpass2");
    table->vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)load(context, "vkCreateRenderPass2");
    table->vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)load(context, "vkGetBufferDeviceAddress");
    table->vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)load(context, "vkGetBufferOpaqueCaptureAddress");
    table->vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)load(context, "vkGetDeviceMemoryOpaqueCaptureAddress");
    table->vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)load(context, "vkGetSemaphoreCounterValue");
    table->vkResetQueryPool = (PFN_vkResetQueryPool)load(context, "vkResetQueryPool");
    table->vkSignalSemaphore = (PFN_vkSignalSemaphore)load(context, "vkSignalSemaphore");
    table->vkWaitSemaphores = (PFN_vkWaitSemaphores)load(context, "vkWaitSemaphores");
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
    table->vkCmdBeginRendering = (PFN_vkCmdBeginRendering)load(context, "vkCmdBeginRendering");
    table->vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)load(context, "vkCmdBindVertexBuffers2");
    table->vkCmdBlitImage2 = (PFN_vkCmdBlitImage2)load(context, "vkCmdBlitImage2");
    table->vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)load(context, "vkCmdCopyBuffer2");
    table->vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)load(context, "vkCmdCopyBufferToImage2");
    table->vkCmdCopyImage2 = (PFN_vkCmdCopyImage2)load(context, "vkCmdCopyImage2");
    table->vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)load(context, "vkCmdCopyImageToBuffer2");
    table->vkCmdEndRendering = (PFN_vkCmdEndRendering)load(context, "vkCmdEndRendering");
    table->vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)load(context, "vkCmdPipelineBarrier2");
    table->vkCmdResetEvent2 = (PFN_vkCmdResetEvent2)load(context, "vkCmdResetEvent2");
    table->vkCmdResolveImage2 = (PFN_vkCmdResolveImage2)load(context, "vkCmdResolveImage2");
    table->vkCmdSetCullMode = (PFN_vkCmdSetCullMode)load(context, "vkCmdSetCullMode");
    table->vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)load(context, "vkCmdSetDepthBiasEnable");
    table->vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)load(context, "vkCmdSetDepthBoundsTestEnable");
    table->vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)load(context, "vkCmdSetDepthCompareOp");
    table->vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)load(context, "vkCmdSetDepthTestEnable");
    table->vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)load(context, "vkCmdSetDepthWriteEnable");
    table->vkCmdSetEvent2 = (PFN_vkCmdSetEvent2)load(context, "vkCmdSetEvent2");
    table->vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace)load(context, "vkCmdSetFrontFace");
    table->vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)load(context, "vkCmdSetPrimitiveRestartEnable");
    table->vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)load(context, "vkCmdSetPrimitiveTopology");
    table->vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)load(context, "vkCmdSetRasterizerDiscardEnable");
    table->vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)load(context, "vkCmdSetScissorWithCount");
    table->vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp)load(context, "vkCmdSetStencilOp");
    table->vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)load(context, "vkCmdSetStencilTestEnable");
    table->vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)load(context, "vkCmdSetViewportWithCount");
    table->vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2)load(context, "vkCmdWaitEvents2");
    table->vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)load(context, "vkCmdWriteTimestamp2");
    table->vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)load(context, "vkCreatePrivateDataSlot");
    table->vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)load(context, "vkDestroyPrivateDataSlot");
    table->vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)load(context, "vkGetDeviceBufferMemoryRequirements");
    table->vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)load(context, "vkGetDeviceImageMemoryRequirements");
    table->vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements)load(context, "vkGetDeviceImageSparseMemoryRequirements");
    table->vkGetPrivateData = (PFN_vkGetPrivateData)load(context, "vkGetPrivateData");
    table->vkQueueSubmit2 = (PFN_vkQueueSubmit2)load(context, "vkQueueSubmit2");
    table->vkSetPrivateData = (PFN_vkSetPrivateData)load(context, "vkSetPrivateData");
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_KHR_swapchain)
    table->vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)load(context, "vkAcquireNextImageKHR");
    table->vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)load(context, "vkCreateSwapchainKHR");
    table->vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)load(context, "vkDestroySwapchainKHR");
    table->vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)load(context, "vkGetSwapchainImagesKHR");
    table->vkQueuePresentKHR = (PFN_vkQueuePresentKHR)load(context, "vkQueuePresentKHR");
#endif /* defined(VK_KHR_swapchain) */
}

/**
 * Copyright (c) 2018-2023 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
