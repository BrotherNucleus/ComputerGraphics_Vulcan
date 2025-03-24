#include <chrono>
#include <thread>

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HAS_SPACESHIP_OPERATOR
#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <random>
#include <functional>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<glm/glm.hpp>
//#include<glm/gtc/quaternion.hpp>
#include<glm/gtc/matrix_transform.hpp>

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

	std::vector<const char*> requiredInstanceExtensions = {
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	uint32_t windowExtensionCount;
	const char** windowExtensions = glfwGetRequiredInstanceExtensions(&windowExtensionCount);

	for (uint32_t i = 0;i < windowExtensionCount;i++)
	{
		requiredInstanceExtensions.push_back(windowExtensions[i]);
	}

	instanceCreateInfo.enabledExtensionCount = (uint32_t)requiredInstanceExtensions.size();
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
			};

	//TLAS

	auto geometryTLAS = vk::AccelerationStructureGeometryKHR{
		.geometryType = vk::GeometryTypeKHR::eInstances,
		.geometry = vk::AccelerationStructureGeometryDataKHR{
		.instances = vk::AccelerationStructureGeometryInstancesDataKHR{
		.arrayOfPointers = false
		}
		},
		.flags = vk::GeometryFlagBitsKHR::eOpaque,
		};

	auto buildInfoTLAS = vk::AccelerationStructureBuildGeometryInfoKHR{.type =vk::AccelerationStructureTypeKHR::eTopLevel,
		.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		.mode = vk::BuildAccelerationStructureModeKHR::eBuild,
		.srcAccelerationStructure = nullptr,
		.dstAccelerationStructure = nullptr,
		.geometryCount = 1,
		.pGeometries = &geometryTLAS,
		.scratchData = {}
		};

	auto buildSizesInfoTLAS = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfoTLAS, { 1 }, dynamicDispatchLoader);

	VulkanAccelerationStructure topAccelerationStructure;

	topAccelerationStructure.structureBuffer = createBuffer(buildSizesInfoTLAS.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	topAccelerationStructure.scratchBuffer = createBuffer(buildSizesInfoTLAS.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	//Createallocationstructure

	auto createInfoTLAS = vk::AccelerationStructureCreateInfoKHR{
		.buffer = topAccelerationStructure.structureBuffer.buffer,
		.offset = 0,
		.size = buildSizesInfoTLAS.accelerationStructureSize,
		.type = vk::AccelerationStructureTypeKHR::eTopLevel
		};

	topAccelerationStructure.accelerationStructure = device.createAccelerationStructureKHR(createInfoTLAS, nullptr, dynamicDispatchLoader);
	vk::TransformMatrixKHR vktransformMatrix;

	memcpy(&vktransformMatrix.matrix, &transformMatrix.matrix, sizeof(transformMatrix));

	auto accelerationStructureInstance = vk::AccelerationStructureInstanceKHR{
		.transform = vktransformMatrix,
		.instanceCustomIndex = 0,
		.mask = 0xFF,
		.instanceShaderBindingTableRecordOffset = 0,
		.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
		//vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,
		};

	accelerationStructureInstance.accelerationStructureReference = device.getAccelerationStructureAddressKHR({
		.accelerationStructure =bottomAccelerationStructure.accelerationStructure}, dynamicDispatchLoader);

	topAccelerationStructure.instancesBuffer = createBuffer(sizeof(vk::AccelerationStructureInstanceKHR),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

	void* pInstancesBuffer = device.mapMemory(topAccelerationStructure.instancesBuffer.memory, 0,
		sizeof(vk::AccelerationStructureInstanceKHR));
	memcpy(pInstancesBuffer, &accelerationStructureInstance, sizeof(vk::AccelerationStructureInstanceKHR));
	device.unmapMemory(topAccelerationStructure.instancesBuffer.memory);

	buildInfoTLAS.dstAccelerationStructure = topAccelerationStructure.accelerationStructure;

	buildInfoTLAS.scratchData.deviceAddress = device.getBufferAddress({
		.buffer =topAccelerationStructure.scratchBuffer.buffer
		});

	geometryTLAS.geometry.instances.data.deviceAddress = device.getBufferAddress({
		.buffer = topAccelerationStructure.instancesBuffer.buffer
		});
	//Buildtheaccelerationstructure
	auto buildRangeInfoTLAS = vk::AccelerationStructureBuildRangeInfoKHR{
		.primitiveCount = 1,
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0
		};
	const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfosTLAS[] = { & buildRangeInfoTLAS};
	[&device, &commandPool, &computePresentQueue, &buildInfoTLAS, &pBuildRangeInfosTLAS, &dynamicDispatchLoader]()
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
			singleTimeCommandBuffer.buildAccelerationStructuresKHR(1, &buildInfoTLAS, pBuildRangeInfosTLAS, dynamicDispatchLoader);
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

	//screen output setup

	struct {
		uint32_t windowWidth;
		uint32_t windowHeight;
	} settings{ .windowWidth = 640, .windowHeight = 480 };

	auto createImageView = [&device](const vk::Image& image, const vk::Format& format) {
		return device.createImageView(
			{
				.image = image,
				.viewType = vk::ImageViewType::e2D,
				.format = format,
				.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
					}
			});
		};

	struct VulkanImage {
		vk::Image image;
		vk::DeviceMemory memory;
		vk::ImageView imageView;
	};

	auto createImage = [&createImageView, &findMemoryTypeIndex, &settings, &device, &physicalDevice]
						(const vk::Format& format, const vk::Flags<vk::ImageUsageFlagBits>& usageFlagBits) {
		vk::ImageCreateInfo imageCreateInfo = {
			.imageType = vk::ImageType::e2D,
			.format = format,
			.extent = {
				.width = settings.windowWidth,
				.height = settings.windowHeight,
				.depth = 1
				},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = usageFlagBits,
			.sharingMode = vk::SharingMode::eExclusive,
			.initialLayout = vk::ImageLayout::eUndefined
			};

		vk::Image image = device.createImage(imageCreateInfo);

		vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocateInfo = {
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
			};

		vk::DeviceMemory memory = device.allocateMemory(allocateInfo);

		device.bindImageMemory(image, memory, 0);

		return VulkanImage{
			.image = image,
			.memory = memory,
			.imageView = createImageView(image, format)
			};
		};

	//Create Window
	GLFWwindow* window = glfwCreateWindow(settings.windowWidth, settings.windowHeight, "Ray Tracing (Vulkan)", nullptr, nullptr);
	if (!window) {
		std::cout << "Shiba" << std::endl;
	}

	//CreateSurface
	vk::SurfaceKHR surface;
	VkResult rres = glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));
	if (rres != VK_SUCCESS) {
		std::cout << "Shiba" << std::endl;
	}
	const uint32_t imageCount = 3;
	const vk::Format swapChainImageFormat = vk::Format::eB8G8R8A8Unorm;//vk::Format::eR8G8B8A8Unorm;
	//CreateSwapChain
	vk::SwapchainKHR swapChain = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR{
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = swapChainImageFormat,//VK_FORMAT_B8G8R8A8_UNORM
		.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
		.imageExtent = {.width = settings.windowWidth,.height = settings.windowHeight},
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = physicalDevice.getSurfaceCapabilitiesKHR(surface).currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = vk::PresentModeKHR::eFifo,//vk::PresentModeKHR::eImmediate
		.clipped = true,
		.oldSwapchain = nullptr
		});

	//swapchainimages
	vk::ImageView swapChainImageViews[imageCount];
	std::vector<vk::Image>swapChainImages = device.getSwapchainImagesKHR(swapChain);
	for (int nn = 0;nn < imageCount;nn++)
	{
		auto image = swapChainImages[nn];
		swapChainImageViews[nn] = createImageView(image, swapChainImageFormat);
	}
	
	//Create Images
	VulkanImage renderTargetImage = createImage(swapChainImageFormat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc);

	//Descriptors

	vk::DescriptorSet rtDescriptorSet;
	VulkanBuffer uniformBuffer;

	[&device, &settings, &createBuffer, &renderTargetImage, &topAccelerationStructure, &rtDescriptorSet, &uniformBuffer]()
		{
			struct UniformData
			{
				glm::mat4 viewInverse;
				glm::mat4 projInverse;
			};
			UniformData uniformData{};
			uniformData.projInverse = glm::inverse(glm::perspective(glm::radians(60.0f), (float)settings.windowWidth / (float)settings.windowHeight, 0.1f, 1000.0f));
			uniformData.viewInverse = glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, -2.5), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));

			const vk::DeviceSize uniformBufferSize = sizeof(uniformData);

			uniformBuffer = createBuffer(uniformBufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal, &uniformData);

			std::vector<vk::DescriptorSetLayoutBinding> bindings = {
				{.binding = 0, .descriptorType = vk::DescriptorType::eStorageImage, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR},
				{.binding = 1, .descriptorType = vk::DescriptorType::eAccelerationStructureKHR, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR},
				{.binding = 2, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR},
				};

			vk::DescriptorSetLayout rtDescriptorSetLayout = device.createDescriptorSetLayout({ .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data() });

			std::vector<vk::DescriptorPoolSize> poolSizes = {
				{.type = vk::DescriptorType::eStorageImage, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eAccelerationStructureKHR, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1 },
			};

			vk::DescriptorPool rtDescriptorPool = device.createDescriptorPool(
				{
				.maxSets = 1,
				.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
				.pPoolSizes = poolSizes.data()
				});

			rtDescriptorSet = device.allocateDescriptorSets(
				{
					.descriptorPool = rtDescriptorPool,
					.descriptorSetCount = 1,
					.pSetLayouts = &rtDescriptorSetLayout
				}).front();

			auto renderTargetImageInfo = vk::DescriptorImageInfo{.imageView = renderTargetImage.imageView,
				 .imageLayout = vk::ImageLayout::eGeneral
				 };
			auto accelerationStructureInfo = vk::WriteDescriptorSetAccelerationStructureKHR{.accelerationStructureCount = 1,
				.pAccelerationStructures = &topAccelerationStructure.accelerationStructure
				};
			auto uniformBufferInfo = vk::DescriptorBufferInfo{.buffer = uniformBuffer.buffer,
				.offset = 0,
				.range = uniformBufferSize
				};

			std::vector<vk::WriteDescriptorSet>descriptorWrites = {
				{.dstSet = rtDescriptorSet,.dstBinding = 0,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eStorageImage, .pImageInfo = &renderTargetImageInfo},
				{.pNext = &accelerationStructureInfo,
				.dstSet = rtDescriptorSet,.dstBinding = 1,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eAccelerationStructureKHR },
				{.dstSet = rtDescriptorSet,.dstBinding = 2,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eUniformBuffer,.pBufferInfo = &uniformBufferInfo}
				};

				device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		};

	return 0;
}