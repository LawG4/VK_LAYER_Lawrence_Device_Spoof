#include "vulkan.h"

#include <stdlib.h>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <vector>

// Are we using validation layers?
bool enableValidation = true;
#ifdef NDEBUG
enableValidation = false;
#endif  // NDEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL deubgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cout << "--------------------------------------------" << std::endl;
        std::cout << "\t* Validation layers message :\n\t" << pCallbackData->pMessage << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
    }
    return VK_FALSE;
}

int main(int argc, char* argv[])
{
    std::string currentDirectory = std::filesystem::current_path().string();
    std::cout << "Using the runtime directory as : \n" << currentDirectory << "\n\n" << std::endl;

    // Place this directory as the VK_LAYER_PATH environment variable
    std::string envCommand = "VK_LAYER_PATH=" + currentDirectory;

    // Append the current VK_LAYER_PATH to not erase anything
    const char* currentLayerPath = getenv("VK_LAYER_PATH");
    if (currentLayerPath) envCommand += ";" + std::string(currentLayerPath);

    // Look in the Vulkan SDK for validation layers as well
    const char* vulkanSDK = getenv("VULKAN_SDK");
    if (vulkanSDK) envCommand += ";" + std::string(vulkanSDK) + "/Bin";

    // Finally place the layer path into the environment variable
    if (putenv(const_cast<char*>(envCommand.c_str())) != 0)
        std::cout << "Failed to set environment variable : " << envCommand << std::endl;

    // It's not guarenteed, but use arg 1 as the working directory for the binary
    std::cout << "==================================================" << std::endl;
    std::cout << "================= Starting Test ==================" << std::endl;
    std::cout << "==================================================" << std::endl;
    // Enumerate the Vulkan instance layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    std::cout << "Found the following layers :" << std::endl;
    for (const VkLayerProperties& layer : layers) {
        std::cout << "\t* " << layer.layerName << std::endl;
    }

    // Search the layers to see if we have support
    std::vector<const char*> layerNames = {"VK_LAYER_Lawrence_Device_Spoof"};
    if (enableValidation) layerNames.push_back("VK_LAYER_KHRONOS_validation");

    for (uint32_t i = 0; i < layerNames.size(); i++) {
        bool layerFound = false;
        for (const VkLayerProperties& prop : layers) {
            if (!strcmp(layerNames[i], prop.layerName))  // found the layer
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)  // Couldn't find this layer in all the layers
        {
            std::cout << "Layer not found : " << layerNames[i] << std::endl;
            if (!strcmp(layerNames[i], "VK_LAYER_KHRONOS_validation"))  // The not found layer is validation
            {
                std::cout << "Disabling validation features" << std::endl;
                enableValidation = false;
            }
            layerNames.erase(layerNames.begin() + i);
            i--;
        }
    }

    // Create an instance that loads in the fake layer
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    info.ppEnabledLayerNames = layerNames.data();
    info.enabledLayerCount = layerNames.size();

    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.apiVersion = VK_VERSION_1_0;
    app.applicationVersion = 0;
    app.engineVersion = 0;
    app.pApplicationName = "Layer Test App";
    app.pEngineName = "None";
    info.pApplicationInfo = &app;

    // No extensions unless we're using validation
    std::vector<const char*> instanceExtensions{};
    if (enableValidation) instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    info.enabledExtensionCount = instanceExtensions.size();
    info.ppEnabledExtensionNames = instanceExtensions.data();

    // Create the debug messenger struct
    VkDebugUtilsMessengerCreateInfoEXT messenger{};
    messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messenger.pfnUserCallback = deubgCallback;
    messenger.pUserData = nullptr;

    if (enableValidation) info.pNext = &messenger;

    VkInstance instance;
    if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS) {
        std::cout << "Failed to create Vulkan instance!" << std::endl;
    } else {
        std::cout << "Created Vulkan instance!" << std::endl;
    }

    // Now do some other code with the instance to ensure it works
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    std::cout << "Displaying physical device properties" << std::endl;

    VkPhysicalDeviceProperties deviceProps{};
    for (const VkPhysicalDevice& device : physicalDevices) {
        vkGetPhysicalDeviceProperties(device, &deviceProps);

        std::cout << "--------------------------------------------------" << std::endl;
        std::cout << deviceProps.deviceName << std::endl;
        std::cout << "\t* Vendor ID      : " << deviceProps.vendorID << std::endl;
        std::cout << "\t* API Version    : " << VK_VERSION_MAJOR(deviceProps.apiVersion) << "."
                  << VK_VERSION_MINOR(deviceProps.apiVersion) << "."
                  << VK_VERSION_PATCH(deviceProps.apiVersion) << std::endl;
        std::cout << "\t* Driver Version : " << VK_VERSION_MAJOR(deviceProps.driverVersion) << "."
                  << VK_VERSION_MINOR(deviceProps.driverVersion) << "."
                  << VK_VERSION_PATCH(deviceProps.driverVersion) << std::endl;
    }
    std::cout << "--------------------------------------------------" << std::endl;
}
