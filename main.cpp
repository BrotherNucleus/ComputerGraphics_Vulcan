#include <chrono>
#include <thread>

#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#include <iostream>

int main() {
	vk::InstanceCreateInfo instanceCreateInfo;
	vk::Instance instance = vk::createInstance(instanceCreateInfo);

	uint32_t apiVersion = vk::enumerateInstanceVersion();
	uint32_t major = VK_VERSION_MAJOR(apiVersion);
	uint32_t minor = VK_VERSION_MINOR(apiVersion);
	uint32_t patch = VK_VERSION_PATCH(apiVersion);

	std::cout << "Vulkan Version: " << major << "." << minor << "." << patch << std::endl;

	instance.destroy();

	return 0;
}