#include <cstring>
#include "vulkan/vk_layer.h"      // For function exporting
#pragma warning(disable : 26812)  // Disable annoying "prefer enum class over enum"

PFN_vkGetInstanceProcAddr vkGetNextInstanceProcAddress = nullptr;
PFN_vkGetDeviceProcAddr vkGetNextDeviceProcAddr = nullptr;
PFN_vkGetPhysicalDeviceProperties vkGetNextPhysicalDeviceProperties = nullptr;

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL
getPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    // Check we retrieved a valid vkPhysicalDeviceProperties struct
    if (!pProperties) return;

    // Call down to the next layer
    if (vkGetNextPhysicalDeviceProperties) vkGetNextPhysicalDeviceProperties(physicalDevice, pProperties);

    // Now edit the results to be something funny
    char deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE] = "NVIDIA GeForce 4050 Beta\0";
    strcpy(pProperties->deviceName, deviceName);

    // Make an unreleased api version
    pProperties->apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL createInstance(const VkInstanceCreateInfo* pCreateInfo,
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
        // The function pointer for getting the instance proc addresses from the next layer is valid
        // Save the function pointers in the next layer down for the functions we plan to intercept

        // vkGetPhysicalDeviceProperties
        vkGetNextPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetNextInstanceProcAddress(
          *pInstance, "vkGetPhysicalDeviceProperties");

        // We don't need to save the next createinstance, and it has to be valid or else we throw error
        PFN_vkCreateInstance createNextInstance =
          (PFN_vkCreateInstance)vkGetNextInstanceProcAddress(*pInstance, "vkCreateInstance");
        if (!createNextInstance) return VK_ERROR_OUT_OF_HOST_MEMORY;

        // Use the next create instance to call down to the next layer
        // Prepare the create info pNext chain for the next layer
        nextChain->u.pLayerInfo = nextChain->u.pLayerInfo->pNext;

        return createNextInstance(pCreateInfo, pAllocator, pInstance);
    }

    // Probably was successful
    return VK_SUCCESS;
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance,
                                                                               const char* funcName)
{
    // If the user is looking for vkGetPhysicalDeviceProperties then intercept it
    // This is the function that we want to change with our layer
    if (!strcmp(funcName, "vkGetPhysicalDeviceProperties"))
        return (PFN_vkVoidFunction)getPhysicalDeviceProperties;

    // If the loader is looking for vkCreateInstance, then tell them about our version
    // This is because when the instance is created we take that opertunity to set up the
    // dispatch chain, so our layer can pass calls onto the next layer
    if (!strcmp(funcName, "vkCreateInstance")) return (PFN_vkVoidFunction)createInstance;

    // Intercept the vkGetInstanceProcAddr as well, this is so this function doesn't get overwritten by the
    // next layer
    if (!strcmp(funcName, "vkGetInstanceProcAddr")) return (PFN_vkVoidFunction)vkGetInstanceProcAddr;

    // Else call down to the next layer
    if (!vkGetNextInstanceProcAddress) return nullptr;
    return vkGetNextInstanceProcAddress(instance, funcName);
}

/**
 * For some reason, and I am not sure why. Using the NegotiateLayerInterfaceVersion causes a hang on linux
 * swiftshader. This seems to revolve around a loader mutex trying to lock, and it is never released
 * I don't know why this happens, but I also had to change the name of the intercepted functions to remove vk
 * except for getInstanceProcAddr, I really am clueless here. Perhaps because this project also builds the
 * loader the function declarations don't agree?
 *
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
        pVersionStruct->pfnGetDeviceProcAddr = nullptr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;

    }

    // Got to the end okay
    return VK_SUCCESS;
}
*/
