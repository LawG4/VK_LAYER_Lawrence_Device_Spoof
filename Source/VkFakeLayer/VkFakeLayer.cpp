#include "vulkan/vk_layer.h"  // For function exporting

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance,
                                                                               const char* funcName)
{
    return nullptr;
}
