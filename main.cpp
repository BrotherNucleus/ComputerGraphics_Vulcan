#include <chrono>
#include <thread>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#include <Windows.h>
#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <random>
#include <functional>

#ifndef DBG_ASSERT
#if defined(_WIN32)
#define DBG_ASSERT(f) {if(!(f)){__debugbreak();};}
#else
#define DBG_ASSERT(f) { #error(platform assert todo) }
#endif
#endif

#define VK_CHECK_RESULT(f) { DBG_ASSERT(f==vk::Result(0)); }
#define DBG_ASSERT_WARN(f, w) { { if (!f) { std::cout << w; }};DBG_ASSERT(f); }

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

	vk::DispatchLoaderDynamic dynamicDispatchLoader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr, device);

	vk::Queue computePresentQueue = device.getQueue(queueId, 0);

	vk::CommandPoolCreateInfo tempCommandPoolInfo;
	tempCommandPoolInfo.queueFamilyIndex = queueId;

	vk::CommandPool commandPool = device.createCommandPool(tempCommandPoolInfo);

	auto findMemoryTypeIndex = [&physicalDevice](const uint32_t& memoryTypeBits, const vk::MemoryPropertyFlags& properties) {
		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; memoryProperties.memoryTypeCount; i++) {
			if ((memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		DBG_ASSERT_WARN(0, "Unable to find suitable memory type!");
		return uint32_t(0);
	};

	struct VulkanBuffer {
		vk::Buffer			buffer;
		vk::DeviceMemory	memory;
		vk::DeviceAddress	address;
	};

	auto createBuffer = [&findMemoryTypeIndex, &physicalDevice, &device](const vk::DeviceSize& size,
		const vk::Flags<vk::BufferUsageFlagBits>& usage,
		const vk::Flags<vk::MemoryPropertyFlagBits>& memoryProperty,
		const void* data = nullptr)
		{
			vk::BufferCreateInfo tempBufferInfo;
			tempBufferInfo.size = size;
			tempBufferInfo.usage = usage;
			tempBufferInfo.sharingMode = vk::SharingMode::eExclusive;
			vk::Buffer buffer = device.createBuffer(tempBufferInfo);

			vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);
			
			vk::MemoryAllocateFlagsInfo allocateFlagsInfo;
			allocateFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;

			vk::MemoryAllocateInfo allocateInfo;
			allocateInfo.pNext = &allocateFlagsInfo;
			allocateInfo.allocationSize = memoryRequirements.size;
			allocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperty);

			vk::DeviceMemory memory = device.allocateMemory(allocateInfo);
			device.bindBufferMemory(buffer, memory, 0);

			if (data)
			{
				void* mappedMemory = device.mapMemory(memory, 0, size);
				memcpy(mappedMemory, data, size);
				device.unmapMemory(memory);
			}

			vk::BufferDeviceAddressInfo tempBufferDeviceAddressInfo;
			tempBufferDeviceAddressInfo.buffer = buffer;

			VulkanBuffer tempVulkanBuffer;
			tempVulkanBuffer.buffer = buffer;
			tempVulkanBuffer.memory = memory;
			tempVulkanBuffer.address = device.getBufferAddress(tempBufferDeviceAddressInfo);

			return tempVulkanBuffer;
		};

	//BLAS - Bottom Level Acceleration Structure (Verts/Tris)

	const uint32_t numTriangles = 1;

	struct Vertex {
		float pos[3];
	};
	const std::vector<Vertex> vertices = {
		{{ 1.0f, 1.0f, 0.0f } },
		{{ -1.0f, 1.0f, 0.0f} },
		{{ 0.0f, -1.0f, 0.0f} }
	};

	std::vector<uint32_t> indeces = { 0, 1, 2 };
	uint32_t indexCount = static_cast<uint32_t>(indeces.size());

	const VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f };

	const vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
	const vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal;

	VulkanBuffer vertexBuffer = createBuffer(vertices.size() * sizeof(Vertex), usageFlags, memoryFlags, vertices.data());
	VulkanBuffer indexBuffer = createBuffer(indeces.size() * sizeof(uint32_t), usageFlags, memoryFlags, indeces.data());
	VulkanBuffer transformBuffer = createBuffer(sizeof(VkTransformMatrixKHR), usageFlags, memoryFlags, &transformMatrix);

	vk::DeviceOrHostAddressConstKHR vertexBufferDeviceAddress;
	vertexBufferDeviceAddress.deviceAddress = vertexBuffer.address;	
	
	vk::DeviceOrHostAddressConstKHR indexBufferDeviceAddress;
	indexBufferDeviceAddress.deviceAddress = indexBuffer.address;	
	
	vk::DeviceOrHostAddressConstKHR transformBufferDeviceAddress;
	transformBufferDeviceAddress.deviceAddress = transformBuffer.address;

	vk::AccelerationStructureGeometryKHR geometry = {
		.geometryType = vk::GeometryTypeKHR::eTriangles,
		.geometry = vk::AccelerationStructureGeometryTrianglesDataKHR{
			.vertexFormat = vk::Format::eR32G32B32A32Sfloat,
			.vertexData = vertexBufferDeviceAddress,
			.vertexStride = sizeof(Vertex),
			.maxVertex = 0,
			.indexType = vk::IndexType::eUint32,
			.indexData = indexBufferDeviceAddress,
			.transformData = transformBufferDeviceAddress
			},
			.flags = vk::GeometryFlagBitsKHR::eOpaque };

	vk::AccelerationStructureBuildGeometryInfoKHR buildInfo = {
		.type = vk::AccelerationStructureTypeKHR::eBottomLevel,
		.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		.mode = vk::BuildAccelerationStructureModeKHR::eBuild,
		.srcAccelerationStructure = nullptr,
		.dstAccelerationStructure = nullptr,
		.geometryCount = 1,
		.pGeometries = &geometry,
		.scratchData = {}
		};

	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = device.getAccelerationStructureBuildSizesKHR(
			vk::AccelerationStructureBuildTypeKHR::eDevice,
			buildInfo,
			numTriangles,
			dynamicDispatchLoader);

	struct VulkanAccelerationStructure{
		 vk::AccelerationStructureKHR accelerationStructure;
		 VulkanBuffer structureBuffer;
		 VulkanBuffer scratchBuffer;
		 VulkanBuffer instancesBuffer;
		 };
	VulkanAccelerationStructure bottomAccelerationStructure;
	//Allocatebuffersforaccelerationstructure
	bottomAccelerationStructure.structureBuffer = createBuffer(buildSizesInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	bottomAccelerationStructure.scratchBuffer = createBuffer(buildSizesInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	//CREATEtheaccelerationsturcture
	vk::AccelerationStructureCreateInfoKHR createInfo = {
	.buffer = bottomAccelerationStructure.structureBuffer.buffer,
	.offset = 0,
	.size = buildSizesInfo.accelerationStructureSize,
	.type = vk::AccelerationStructureTypeKHR::eBottomLevel
	};
	bottomAccelerationStructure.accelerationStructure = device.createAccelerationStructureKHR(createInfo, nullptr, dynamicDispatchLoader);
	//Fillintheremainingmetainfo
	buildInfo.dstAccelerationStructure = bottomAccelerationStructure.accelerationStructure;
	buildInfo.scratchData.deviceAddress = device.getBufferAddress({.buffer =bottomAccelerationStructure.scratchBuffer.buffer});
	//BUILDtheaccelerationstructure
	vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
	.primitiveCount = numTriangles,
	.primitiveOffset = 0,
	.firstVertex = 0,
	.transformOffset = 0
	};

	const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos[] = {&buildRangeInfo};
	[&device, &commandPool, &computePresentQueue, &buildInfo, &pBuildRangeInfos, & dynamicDispatchLoader]()
			{
				vk::CommandBuffer singleTimeCommandBuffer = device.allocateCommandBuffers(
						{
						.commandPool = commandPool,
						.level = vk::CommandBufferLevel::ePrimary,
						.commandBufferCount = 1
						}).front();

				vk::CommandBufferBeginInfo beginInfo = {
				.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
				};
				VK_CHECK_RESULT(singleTimeCommandBuffer.begin(&beginInfo));
				singleTimeCommandBuffer.buildAccelerationStructuresKHR(1, &buildInfo,pBuildRangeInfos, dynamicDispatchLoader);
				singleTimeCommandBuffer.end();
				vk::SubmitInfo submitInfo = {
				.commandBufferCount = 1,
				.pCommandBuffers = &singleTimeCommandBuffer
				};
				vk::Fence f = device.createFence({});
				VK_CHECK_RESULT(computePresentQueue.submit(1, &submitInfo, f));
				VK_CHECK_RESULT(device.waitForFences(1, &f, true, UINT64_MAX));
				device.destroyFence(f);
				device.freeCommandBuffers(commandPool, singleTimeCommandBuffer);
			}();
	return 0;
}