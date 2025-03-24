#include <chrono>
#include <thread>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <functional>

int main() {
	vk::InstanceCreateInfo instanceCreateInfo;
	vk::Instance instance = vk::createInstance(instanceCreateInfo);

	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

	std::cout << "Number of devices: " << physicalDevices.size() << std::endl;

	for (const auto& physicalDevice : physicalDevices) {
		std::vector<vk::ExtensionProperties> extensionProperites = physicalDevice.enumerateDeviceExtensionProperties();
		std::cout << "Extension Properties:" << std::endl;
		for (const auto& extension : extensionProperites)
		{
			std::cout << extension.extensionName << std::endl;
		}
	}

	uint32_t apiVersion = vk::enumerateInstanceVersion();
	uint32_t major = VK_VERSION_MAJOR(apiVersion);
	uint32_t minor = VK_VERSION_MINOR(apiVersion);
	uint32_t patch = VK_VERSION_PATCH(apiVersion);

	std::cout << "Vulkan Version: " << major << "." << minor << "." << patch << std::endl;

	instance.destroy();

	return 0;
}