#include <string>
#include "vulkan/vk_layer.h"      // For function exporting
#pragma warning(disable : 26812)  // Disable annoying "prefer enum class over enum"

PFN_vkGetInstanceProcAddr vkGetNextInstanceProcAddress = nullptr;

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance,
                                                                               const char* funcName)
{
    // If the user is looking for vkCreateInstance, the tell them about our version
    if (!strcmp(funcName, "vkCreateInstance")) return (PFN_vkVoidFunction)vkCreateInstance;

    if (!vkGetNextInstanceProcAddress) return nullptr;
    return vkGetNextInstanceProcAddress(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkInstance* pInstance)
{
    // Let's loop through all of the pNext chains, to find the next VkLayerInstanceCreateInfo
    // this tells us all of the information we need to know about the next layer
    VkLayerInstanceCreateInfo* nextChain = (VkLayerInstanceCreateInfo*)pCreateInfo->pNext;
    while (nextChain && !(nextChain->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
                          nextChain->function == VK_LAYER_LINK_INFO)) {
        // Loop through the pNext chains until we've found the layer instance create info
        // for the next layer in the dispatch table
        nextChain = (VkLayerInstanceCreateInfo*)nextChain->pNext;
    }

    // If this there is not anoter element in the chain, we know we're the last layer
    if (!nextChain) return VK_SUCCESS;  // Probably successful?

    // Check that the element we recieved via pNext chain was valid
    if (!nextChain->u.pLayerInfo) return VK_ERROR_OUT_OF_HOST_MEMORY;

    // We've found the next layer! keep the dispatch at hand
    vkGetNextInstanceProcAddress = nextChain->u.pLayerInfo->pfnNextGetInstanceProcAddr;

    if (vkGetNextInstanceProcAddress) {
        // We have another layer in the dispatch chain
        PFN_vkCreateInstance createNextInstance =
          (PFN_vkCreateInstance)vkGetNextInstanceProcAddress(*pInstance, "vkCreateInstance");
        if (!createNextInstance) return VK_ERROR_OUT_OF_HOST_MEMORY;

        // Prepare the create info pNext chain for the next layer
        nextChain->u.pLayerInfo = nextChain->u.pLayerInfo->pNext;

        return createNextInstance(pCreateInfo, pAllocator, pInstance);
    }

    // Probably was successful
    return VK_SUCCESS;
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
