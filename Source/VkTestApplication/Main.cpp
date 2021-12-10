#include "vulkan.h"

#include <filesystem>
#include <vector>
#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	std::string currentDirectory = std::filesystem::current_path().string();
	std::cout << "Using the runtime directory as : \n" << currentDirectory << "\n\n" << std::endl;

	// Place this directory as the VK_LAYER_PATH environment variable
	std::string envCommand = "VK_LAYER_PATH=" + currentDirectory;
	putenv(envCommand.c_str());

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
	for (const VkLayerProperties& layer : layers)
	{
		std::cout << "\t* " << layer.layerName << std::endl;
	}
	
}