#include "vulkan/vk_layer.h"  // For function exporting

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance,
                                                                               const char* funcName)
{
    return nullptr;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct)
{
    // How we want to tell the Vulkan loader how our layer works
    // First ensure we've recieved the right struct
    if (!pVersionStruct) return VK_ERROR_INITIALIZATION_FAILED;
    if (pVersionStruct->sType != LAYER_NEGOTIATE_INTERFACE_STRUCT) return VK_ERROR_INITIALIZATION_FAILED;

    // What version of the struct is the loader using?
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    }

    // Got to the end okay
    return VK_SUCCESS;
}
