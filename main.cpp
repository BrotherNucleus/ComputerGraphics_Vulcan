#include <chrono>
#include <thread>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <random>
#include <functional>

int main() {
	vk::InstanceCreateInfo instanceCreateInfo;

	const std::vector<const char*> requiredInstanceExtensions = {
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};
	instanceCreateInfo.enabledExtensionCount = requiredInstanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

	vk::Instance instance = vk::createInstance(instanceCreateInfo);

	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

	uint32_t apiVersion = vk::enumerateInstanceVersion();
	uint32_t major = VK_VERSION_MAJOR(apiVersion);
	uint32_t minor = VK_VERSION_MINOR(apiVersion);
	uint32_t patch = VK_VERSION_PATCH(apiVersion);

	std::cout << "Vulkan Version: " << major << "." << minor << "." << patch << std::endl;

	std::cout << "Number of devices: " << physicalDevices.size() << std::endl;

	auto getDeviceProperties = [](const vk::PhysicalDevice& physicalDevice)
		{
			vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
			return deviceProperties;
		};

	auto getRayTracingFeatures = [](const vk::PhysicalDevice& physicalDevice)
		{
			vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
			vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
			physicalDeviceFeatures2.pNext = &rayTracingFeatures;
			physicalDevice.getFeatures2(&physicalDeviceFeatures2);
			return rayTracingFeatures;
		};

	auto getRayTracingProperties = [](const vk::PhysicalDevice& physicalDevice)
		{
			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelinePropertiesKhr = {};
			vk::PhysicalDeviceProperties2 physicalDeviceProperties2 = {};
			physicalDeviceProperties2.pNext = &rayTracingPipelinePropertiesKhr;
			physicalDevice.getProperties2(&physicalDeviceProperties2);
			return rayTracingPipelinePropertiesKhr;
		};

	auto getAccStructureFeatures = [](const vk::PhysicalDevice& physicalDevice)
		{
			vk::PhysicalDeviceAccelerationStructureFeaturesKHR accStructureFeatures = {};
			vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
			physicalDeviceFeatures2.pNext = &accStructureFeatures;
			physicalDevice.getFeatures2(&physicalDeviceFeatures2);
			return accStructureFeatures;
		};

	auto getAccStructureProperties = [](const vk::PhysicalDevice& physicalDevice)
		{
			vk::PhysicalDeviceAccelerationStructurePropertiesKHR accStructureProperties = {};
			vk::PhysicalDeviceProperties2 physicalDeviceProperties2 = {};
			physicalDeviceProperties2.pNext = &accStructureProperties;
			physicalDevice.getProperties2(&physicalDeviceProperties2);
			return accStructureProperties;
		};

	auto getRayQueryFeatures = [](const vk::PhysicalDevice& physicalDevice)
		{
			vk::PhysicalDeviceRayQueryFeaturesKHR accRayQueryFeatures = {};
			vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
			physicalDeviceFeatures2.pNext = &accRayQueryFeatures;
			physicalDevice.getFeatures2(&physicalDeviceFeatures2);
			return accRayQueryFeatures;
		};

	//for (const auto& physicalDevice : physicalDevices) {
	//	std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
	//	vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();

	//	if (getRayTracingFeatures(physicalDevice).rayTracingPipeline)
	//	{
	//		std::cout << "Raytracing extension is supported on device: " << deviceProperties.deviceName << std::endl;
	//		std::cout << "Max Rec Depth: " << getRayTracingProperties(physicalDevice).maxRayRecursionDepth << std::endl;
	//		std::cout << "Has Acceleration Structure: " << getAccStructureFeatures(physicalDevice).accelerationStructure << std::endl;
	//		std::cout << "Max Prim Count: " << getAccStructureProperties(physicalDevice).maxGeometryCount << std::endl;
	//		std::cout << "Has Ray Query: " << getRayQueryFeatures(physicalDevice).rayQuery << std::endl;
	//	}
	//	else
	//	{
	//		std::cout << "Raytracing extension is not supported on device:"<<deviceProperties.deviceName<<std::endl;
	//	}
	//}

	const std::vector<const char*> requiredDeviceExtensions = {
		 VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		 VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		 VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		 VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		 VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		 VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		 VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		 VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		 VK_KHR_MAINTENANCE3_EXTENSION_NAME
	};

	vk::PhysicalDevice physicalDevice = nullptr;

	for (const vk::PhysicalDevice& d : physicalDevices) {
		std::vector<vk::ExtensionProperties> availableExtensions = d.enumerateDeviceExtensionProperties();
		std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

		for (const vk::ExtensionProperties& extention : availableExtensions)
		{
			requiredExtensions.erase(extention.extensionName);
		}
		if (requiredExtensions.empty()) {
			physicalDevice = d;
			break;
		}
	}

	std::cout << "Selecteddevice:" << getDeviceProperties(physicalDevice).deviceName << std::endl;
	std::cout << "Raytracingsupported:" << getRayTracingFeatures(physicalDevice).rayTracingPipeline << std::endl;
	std::cout << "MaxRecDepth:" << getRayTracingProperties(physicalDevice).maxRayRecursionDepth << std::endl;
	std::cout << "HasAccelerationStructure:" << getAccStructureFeatures(physicalDevice).accelerationStructure << std::endl;
	std::cout << "MaxPrimCount:" << getAccStructureProperties(physicalDevice).maxGeometryCount << std::endl;
	std::cout << "HasRayQuery:" << getRayQueryFeatures(physicalDevice).rayQuery << std::endl;

	uint32_t queueId = [&physicalDevice]() 
		{
			std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
			for (uint32_t i = 0; i < queueFamilies.size(); i++) {
				bool supportsGraphics = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
				bool supportsCompute = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute;
				if (supportsCompute && supportsGraphics) {
					return i;
				}
			}
			std::cerr << "Unable to find a queue that supports both compute and graphic family"<<std::endl;
			return (uint32_t)-1;
		}();

	float queuePriority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	vk::DeviceQueueCreateInfo tempQueueInfo;
	tempQueueInfo.queueFamilyIndex = queueId;
	tempQueueInfo.queueCount = 1;
	tempQueueInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(tempQueueInfo);

	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
	bufferDeviceAddressFeatures.bufferDeviceAddress = true;
	bufferDeviceAddressFeatures.bufferDeviceAddressCaptureReplay = false;
	bufferDeviceAddressFeatures.bufferDeviceAddressMultiDevice = false;


	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures;
	rayTracingPipelineFeatures.pNext = &bufferDeviceAddressFeatures;
	rayTracingPipelineFeatures.rayTracingPipeline = true;

	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
	accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
	accelerationStructureFeatures.accelerationStructure = true;
	accelerationStructureFeatures.accelerationStructureCaptureReplay = true;
	accelerationStructureFeatures.accelerationStructureIndirectBuild = false;
	accelerationStructureFeatures.accelerationStructureHostCommands = false;
	accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = false;

	vk::DeviceCreateInfo tempDeviceInfo;
	tempDeviceInfo.pNext = &accelerationStructureFeatures;
	tempDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	tempDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	tempDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	tempDeviceInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
	tempDeviceInfo.pEnabledFeatures = nullptr;
	
	auto deviceCreateInfo = tempDeviceInfo;

	vk::Device device = physicalDevice.createDevice(deviceCreateInfo);


	instance.destroy();

	return 0;
}