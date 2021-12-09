#include "vulkan.h"
#include <vector>
#include <iostream>

int main(int argc, char* argv[])
{
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