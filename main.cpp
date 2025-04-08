#include <chrono>
#include <thread>

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HAS_SPACESHIP_OPERATOR
#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#include<shaderc/shaderc.hpp>
#pragma comment(lib, "shadercd.lib")
#pragma comment(lib, "shaderc_utild.lib")
#pragma comment(lib, "shaderc_combinedd.lib")

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <random>
#include <functional>
#include <vector>

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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

const std::string raygenShaderCode = R"(
 #version 460
 #extension GL_EXT_ray_tracing : enable

 layout(binding=0,set=0,rgba8) uniform image2D image;
 layout(binding=1,set=0) uniform accelerationStructureEXT topLevelAS;
 layout(binding=2,set=0) uniform CameraProperties
 {
	 mat4 view;
	 mat4 proj;
	 int samples;
 }cam;

struct stHitValue {
 vec3 color;
 bool miss;
 };

 layout(location=0) rayPayloadEXT stHitValue hitValue;

vec3 traceRay(vec2 d) {

 vec4 origin = inverse(cam.view) * vec4(0,0,0,1);
 vec4 target = inverse(cam.proj) * vec4(d.x, d.y, 1, 1) ;
 vec4 direction = inverse(cam.view)*vec4(normalize(target.xyz), 0) ;

//ortho

//vec4 origin = inverse(cam.view) * inverse(cam.proj) * vec4(d.x, d.y, 0, 1);
//vec4 direction = inverse(cam.view) * vec4(0, 0, -1, 0);

 float tmin = 0.001;
 float tmax = 10000.0;
 hitValue.color = vec3(0.0);
 hitValue.miss = false;
 traceRayEXT(topLevelAS,gl_RayFlagsOpaqueEXT,0xff,0,0,0,origin.xyz,tmin,direction.xyz,tmax,0);
 return hitValue.color;
}

void main()
{
vec3 finalColor = vec3(0.0);

float jitter = 0.5 / cam.samples;

for (int i = 1; i <= cam.samples; i++) 
	{
		for (int j = 1; j <= cam.samples; j++)  
		{
			 const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy)+vec2(i * jitter, j * jitter);
			 const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
			 vec2 d = inUV*2.0-1.0;

			 vec3 color = traceRay(d);
	
			finalColor += color;
		}
	}
finalColor /= (cam.samples*cam.samples);

 imageStore(image,ivec2(gl_LaunchIDEXT.xy),vec4(finalColor,0.0));
 })";

const std::string missShaderCode = R"(
 #version 460
 #extension GL_EXT_ray_tracing : enable

struct stHitValue{
 vec3 color;
 bool miss;
 };

 layout(location=0) rayPayloadInEXT stHitValue hitValue;
 void main()
 {
	hitValue.color = vec3(0.0, 0.0, 0.2);
	hitValue.miss = true;
 })";

const std::string closestHitShaderCode = R"(
 #version 460
 #extension GL_EXT_ray_tracing : enable
 #extension GL_EXT_nonuniform_qualifier : enable

struct stHitValue{
 vec3 color;
 bool miss;
 };

 layout(location=0) rayPayloadInEXT stHitValue hitValue;

 struct DirectionalLight {
	vec3 direction;
	vec3 color;
};

struct BufferVertex {
	float x, y, z;
	};

struct Vertex {
	vec3 pos;
};

struct BufferMat {
	float dx, dy, dz, ax, ay, az, s, sh;
};

struct Material {
	vec3 diffuse;
	vec3 ambient;
	float specular;
	float shininess;
};

struct lightBuffer {
	float dx, dy, dz, cx, cy, cz;
};

layout(binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 3) buffer VertexBuffer {
    BufferVertex vertex_buffer[];
};
layout(set = 0, binding = 4) buffer IndexBuffer {
    int posI[];
};
layout(set = 0, binding = 6) buffer MaterialBuffer {
    BufferMat matBuff;
};
layout(set = 0, binding = 5) uniform Camera {
	 mat4 view;
	 mat4 proj;
	 int samples;
}cam;

layout(set = 0, binding = 7) buffer Light {
	lightBuffer lightBuf;
};
hitAttributeEXT vec2 attribs;

vec3 fetchVertex( int index) {
	int i = posI[index];
	BufferVertex bv = vertex_buffer[i];
	vec3 v;
	v = vec3(bv.x ,bv.y, bv.z);
	return v; 
}

Material fetchMaterial() {
	Material m;
	vec3 dif = vec3(matBuff.dx, matBuff.dy, matBuff.dz);
	vec3 am = vec3(matBuff.ax, matBuff.ay, matBuff.az);
	m.diffuse = dif;
	m.ambient = am;
	m.specular = matBuff.s;
	m.shininess = matBuff.sh;
	return m;
}

DirectionalLight fetchLight() {
	DirectionalLight l;
	l.direction = vec3(lightBuf.dx, lightBuf.dy, lightBuf.dz);
	l.color = vec3(lightBuf.cx, lightBuf.cy, lightBuf.cz);
	return l;
}

 void main()
 {
	int primitiveID = gl_PrimitiveID;

	vec3 v0 = fetchVertex(primitiveID*3 + 0);
	vec3 v1 = fetchVertex(primitiveID*3 + 1);
	vec3 v2 = fetchVertex(primitiveID*3 + 2);

	v0 = gl_ObjectToWorldEXT * vec4(v0, 1);
	v1 = gl_ObjectToWorldEXT * vec4(v1, 1);
	v2 = gl_ObjectToWorldEXT * vec4(v2, 1);

	vec3 normal = normalize(cross(v0 - v1, v0 - v2));
	
	Material mat = fetchMaterial();

	vec3 ambient = mat.ambient;

	DirectionalLight light = fetchLight();
	vec3 lightDirection = normalize(-light.direction);
	float lightIntensity = max(dot(normal, lightDirection), 0.0);


	vec3 diffuse = light.color * lightIntensity;
	vec3 surfaceColor = mat.diffuse;

	float shininess = mat.shininess;

	vec4 origin = inverse(cam.view) * vec4(0, 0, 0, 1);
	vec3 o = vec3(origin.x, origin.y, origin.z);
	vec3 hitPos = vec3(gl_WorldRayOriginEXT + gl_RayTmaxEXT * gl_WorldRayDirectionEXT);
	float specularStrength = mat.specular;
	vec3 viewDir = normalize(o - hitPos);
	vec3 halfwayDir = normalize(lightDirection + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular = specularStrength * spec * light.color;

	 uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT;
	float rayMin = 0.001;
	float rayMax = 10000.0;
	float shadowBias = 0.001;
	uint cullMask = 0xFFu;
	vec3 shadowRayOrigin = hitPos+shadowBias*normal;
	vec3 shadowRayDirection=lightDirection;
	hitValue.miss=false;
	//shotshadowray
	traceRayEXT(topLevelAS,rayFlags,cullMask,0u,0u,0u,shadowRayOrigin,rayMin,shadowRayDirection,rayMax,0);

	float shadow=1.0;
	 if(!hitValue.miss )
	 {
	 shadow=0.1;
	 }

	hitValue.color = surfaceColor * (diffuse + ambient + specular) * shadow;;
	hitValue.miss = false;
 })";

int sampleNumber = 1;

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

	struct DirectionalLight {
		float direction[3];
		float color[3];
	};

	DirectionalLight light = {
		.direction = {0.25f, 0.5f, 0.25f},
		.color = {1.0f, 1.0f, 1.0f}
	};

	struct Material {
		float diffuse[3];
		float ambient[3];
		float specular;
		float shininess;
	};
	float materialSize = sizeof(float) * 8;

	Material base = {
		.diffuse = {0.8, 0.6, 0.4},
		.ambient = {0.2, 0.2, 0.2},
		.specular = 0.9,
		.shininess = 64
	};

	//BLAS - Bottom Level Acceleration Structure (Verts/Tris)

	struct Vertex {
		float pos[3];
	};
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indeces;
	
	const char* filename = "Models/sphere.obj";

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	//std::string warn;
	std::string err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, nullptr);

	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

			int fv = 3;

			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

				Vertex new_vert;
				new_vert.pos[0] = vx;
				new_vert.pos[1] = vy;
				new_vert.pos[2] = vz;

				vertices.push_back(new_vert);
				indeces.push_back(uint32_t(vertices.size() - 1));
			}
			index_offset += fv;
		}
	}

	uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
	std::cout << vertexCount << std::endl;
	uint32_t indexCount = static_cast<uint32_t>(indeces.size());
	const uint32_t numTriangles = indexCount / 3;

	const VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f };

	const vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
	const vk::BufferUsageFlags VusageFlags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eVertexBuffer;
	const vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal;
	const vk::BufferUsageFlags matUsageFlags = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer;

	VulkanBuffer vertexBuffer = createBuffer(vertices.size() * sizeof(Vertex), VusageFlags, memoryFlags, vertices.data());
	VulkanBuffer indexBuffer = createBuffer(indeces.size() * sizeof(uint32_t), usageFlags, memoryFlags, indeces.data());
	VulkanBuffer transformBuffer = createBuffer(sizeof(VkTransformMatrixKHR), usageFlags, memoryFlags, &transformMatrix);
	VulkanBuffer materialBuffer = createBuffer(materialSize, matUsageFlags, memoryFlags, &base);
	VulkanBuffer lightBuffer = createBuffer(sizeof(float)*6, matUsageFlags, memoryFlags, &light);

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

	vktransformMatrix.matrix[0][3] = -1.5f;
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

	VkTransformMatrixKHR vktransformMatrix2 = vktransformMatrix;
	vktransformMatrix2.matrix[0][3] = 1.5f;

	auto accelerationStructureInstance2 = accelerationStructureInstance;
	accelerationStructureInstance2.transform = vktransformMatrix2;
	accelerationStructureInstance2.instanceCustomIndex = 1;

	topAccelerationStructure.instancesBuffer = createBuffer(2 * sizeof(vk::AccelerationStructureInstanceKHR),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

	void* pInstancesBuffer = device.mapMemory(topAccelerationStructure.instancesBuffer.memory, 0,
		2 * sizeof(vk::AccelerationStructureInstanceKHR));

	vk::AccelerationStructureInstanceKHR instances[] = { accelerationStructureInstance, accelerationStructureInstance2 };

	memcpy(pInstancesBuffer, instances, 2 * sizeof(vk::AccelerationStructureInstanceKHR));
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
		.primitiveCount = 2,
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
			.samples = vk::SampleCountFlagBits::e4,
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
	vk::DescriptorSetLayout rtDescriptorSetLayout;
	VulkanBuffer uniformBuffer;

	[&device, &settings, &createBuffer, &renderTargetImage, &topAccelerationStructure, &rtDescriptorSet, &rtDescriptorSetLayout, &uniformBuffer, &vertexBuffer, &indexBuffer, &materialBuffer, &lightBuffer]()
		{
			std::cout << "Lambda Called\n";
			struct UniformData
			{
				//glm::mat4 model;
				glm::mat4 view;
				glm::mat4 proj;
				int samples;
			};
			UniformData uniformData{};
			//uniformData.proj = glm::perspective(glm::radians(60.0f), (float)settings.windowWidth / (float)settings.windowHeight, 0.1f, 1000.0f);
			uniformData.proj = glm::ortho(-(float)settings.windowWidth / 2, (float)settings.windowWidth / 2, -(float)settings.windowHeight / 2, (float)settings.windowHeight / 2, 0.1f, 1000.0f);
			//uniformData.projInverse = glm::inverse(glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f));
			uniformData.view = glm::lookAt(glm::vec3(0.0, 0.0, -2.5), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			//uniformData.model = glm::mat4(1.0f);
			uniformData.samples = sampleNumber;

			const vk::DeviceSize uniformBufferSize = sizeof(uniformData);
			std::cout << "Creating uniform buffer...\n";
			uniformBuffer = createBuffer(uniformBufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal, &uniformData);

			if (!uniformBuffer.buffer) {
				std::cerr << "[Error] uniformBuffer.buffer is NULL! Buffer creation failed.\n";
			}

			if (!uniformBuffer.memory) {
				std::cerr << "[Error] uniformBuffer.memory is NULL! Memory allocation failed.\n";
			}

			std::vector<vk::DescriptorSetLayoutBinding> bindings = {
				{.binding = 0, .descriptorType = vk::DescriptorType::eStorageImage, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR},
				{.binding = 1, .descriptorType = vk::DescriptorType::eAccelerationStructureKHR, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR},
				{.binding = 2, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR},
				{.binding = 3, .descriptorType = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR},
				{.binding = 4, .descriptorType = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR},
				{.binding = 5, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR},
				{.binding = 6, .descriptorType = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR},
				{.binding = 7, .descriptorType = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR},
				};

			rtDescriptorSetLayout = device.createDescriptorSetLayout({ .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data() });

			std::vector<vk::DescriptorPoolSize> poolSizes = {
				{.type = vk::DescriptorType::eStorageImage, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eAccelerationStructureKHR, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1 },
				{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1 },
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
			auto vertexBufferInfo = vk::DescriptorBufferInfo{
				.buffer = vertexBuffer.buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};

			auto indexBufferInfo = vk::DescriptorBufferInfo{
				.buffer = indexBuffer.buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};

			auto materialBufferInfo = vk::DescriptorBufferInfo{
				.buffer = materialBuffer.buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE,
			};

			auto lightBufferInfo = vk::DescriptorBufferInfo{
				.buffer = lightBuffer.buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};

			std::vector<vk::WriteDescriptorSet>descriptorWrites = {
				{.dstSet = rtDescriptorSet,.dstBinding = 0,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eStorageImage, .pImageInfo = &renderTargetImageInfo},
				{.pNext = &accelerationStructureInfo,
				.dstSet = rtDescriptorSet,.dstBinding = 1,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eAccelerationStructureKHR },
				{.dstSet = rtDescriptorSet,.dstBinding = 2,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eUniformBuffer,.pBufferInfo = &uniformBufferInfo},
				{.dstSet = rtDescriptorSet,.dstBinding = 3,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eStorageBuffer,.pBufferInfo = &vertexBufferInfo},
				{.dstSet = rtDescriptorSet,.dstBinding = 4,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eStorageBuffer,.pBufferInfo = &indexBufferInfo},
				{.dstSet = rtDescriptorSet,.dstBinding = 5,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eUniformBuffer,.pBufferInfo = &uniformBufferInfo},
				{.dstSet = rtDescriptorSet,.dstBinding = 6,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eStorageBuffer,.pBufferInfo = &materialBufferInfo},
				{.dstSet = rtDescriptorSet,.dstBinding = 7,.dstArrayElement = 0,.descriptorCount = 1,.descriptorType = vk::DescriptorType::eStorageBuffer,.pBufferInfo = &lightBufferInfo}
				};

				device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}();

	//Shaders

	auto createShaderModule = [&device](const std::string& path) {
		auto readBinaryFile = [](const std::string& path) {
			std::ifstream file(path, std::ios::ate | std::ios::binary);
			DBG_ASSERT_WARN(file.is_open(), "[Error] Failed to open file at'" + path + "'!");

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);
			file.close();
			return buffer;
			};

		std::vector<char> shaderCode = readBinaryFile(path);

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo = {
			.codeSize = shaderCode.size(),
			.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
		};
		return device.createShaderModule(shaderModuleCreateInfo);
		};

	auto createShaderModuleFromGLSL = [&device](const std::string& glslSourceCode, shaderc_shader_kind shaderKind)
			{
				const char* shaderSource = glslSourceCode.c_str();
				//Create a shaderc compiler instance
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;
				//Set the targeted SPIR-V version
				options.SetTargetSpirv(shaderc_spirv_version_1_6); //SetthedesiredSPIR - Vversion
					//Compile the shader sourcecode
					shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderSource,
						strlen(shaderSource),
						shaderKind,
						"shader.rmiss.spv",
						options);
				if (module.GetCompilationStatus()!=shaderc_compilation_status_success) {
					//Handle shader compilation error
					std::cerr << module.GetErrorMessage() << std::endl;
					DBG_ASSERT(0);
				}
				//Retrieve the SPIR-V bytecode from the compilation result
				const auto spirvCode = module.cbegin();
				const size_t spirvSize = (size_t)(module.cend() - module.cbegin()) * 4;//uint32tochar
				//Create a Vulkan shader module
				vk::ShaderModuleCreateInfo createInfo{.codeSize = spirvSize,.pCode =spirvCode};
				return device.createShaderModule(createInfo);
			};

	//Create shader modules from inline
	vk::ShaderModule raygenModule = createShaderModuleFromGLSL(raygenShaderCode, shaderc_shader_kind::shaderc_raygen_shader);
	vk::ShaderModule  chitModule = createShaderModuleFromGLSL(closestHitShaderCode, shaderc_shader_kind::shaderc_closesthit_shader);
	vk::ShaderModule missModule = createShaderModuleFromGLSL(missShaderCode, shaderc_shader_kind::shaderc_miss_shader);

	std::vector<vk::PipelineShaderStageCreateInfo>stages = {
		{.stage = vk::ShaderStageFlagBits::eRaygenKHR, .module = raygenModule,.pName = "main"},
		{.stage = vk::ShaderStageFlagBits::eMissKHR, .module = missModule, .pName = "main"},
		{.stage = vk::ShaderStageFlagBits::eClosestHitKHR, .module = chitModule, .pName = "main"}
		};

	std::vector<vk::RayTracingShaderGroupCreateInfoKHR>groups = {

		{.type = vk::RayTracingShaderGroupTypeKHR::eGeneral, 
		.generalShader = 0, 
		.closestHitShader = VK_SHADER_UNUSED_KHR,
		.anyHitShader = VK_SHADER_UNUSED_KHR,
		.intersectionShader = VK_SHADER_UNUSED_KHR},

		{.type = vk::RayTracingShaderGroupTypeKHR::eGeneral, 
		.generalShader = 1, 
		.closestHitShader = VK_SHADER_UNUSED_KHR,
		.anyHitShader = VK_SHADER_UNUSED_KHR,
		.intersectionShader = VK_SHADER_UNUSED_KHR},


		{.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,
		.generalShader = VK_SHADER_UNUSED_KHR,
		.closestHitShader = 2,
		.anyHitShader = VK_SHADER_UNUSED_KHR,
		.intersectionShader =VK_SHADER_UNUSED_KHR}


		};


	vk::PipelineLayout rtPipelineLayout = device.createPipelineLayout(//vk::PipelineLayout
		{
		.setLayoutCount = 1,
		.pSetLayouts = &rtDescriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
		});
	vk::PipelineLibraryCreateInfoKHR libraryCreateInfo = {.libraryCount = 0};
	vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo = {
		.stageCount = static_cast<uint32_t>(stages.size()),
		.pStages = stages.data(),
		.groupCount = static_cast<uint32_t>(groups.size()),
		.pGroups = groups.data(),
		.maxPipelineRayRecursionDepth = getRayTracingProperties(physicalDevice).maxRayRecursionDepth,
		.pLibraryInfo = &libraryCreateInfo,
		.pLibraryInterface = nullptr,
		.layout = rtPipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0
	};
	vk::Pipeline rtPipeline = device.createRayTracingPipelineKHR(nullptr, nullptr, pipelineCreateInfo, nullptr, dynamicDispatchLoader).value;
	device.destroyShaderModule(raygenModule);
	device.destroyShaderModule(chitModule);
	device.destroyShaderModule(missModule);

	//Create shader Binding Table
	vk::StridedDeviceAddressRegionKHR sbtRayGenAddressRegion;
	vk::StridedDeviceAddressRegionKHR sbtMissAddressRegion;
	vk::StridedDeviceAddressRegionKHR sbtHitAddressRegion;
	[&device, &createBuffer, &rtPipeline, &getRayTracingProperties, &dynamicDispatchLoader, &physicalDevice]
	(vk::StridedDeviceAddressRegionKHR& sbtRayGenAddressRegion,
		vk::StridedDeviceAddressRegionKHR& sbtMissAddressRegion,
		vk::StridedDeviceAddressRegionKHR& sbtHitAddressRegion)
		{
			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties = getRayTracingProperties(physicalDevice);
			uint32_t baseAlignment = rayTracingProperties.shaderGroupBaseAlignment;
			uint32_t handleSize = rayTracingProperties.shaderGroupHandleSize;

			const uint32_t shaderGroupCount = 3;
			vk::DeviceSize sbtBufferSize = baseAlignment * shaderGroupCount;

			VulkanBuffer shaderBindingTableBuffer = createBuffer(sbtBufferSize,
				vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal);

			std::vector<uint8_t> handles = device.getRayTracingShaderGroupHandlesKHR<uint8_t>(rtPipeline, 0, shaderGroupCount, shaderGroupCount * handleSize, dynamicDispatchLoader);

			vk::DeviceAddress sbtAddress = device.getBufferAddress({ .buffer = shaderBindingTableBuffer.buffer });

			sbtRayGenAddressRegion = {
				.deviceAddress = sbtAddress + baseAlignment * 0,
				.stride = baseAlignment,
				.size = baseAlignment
			};

			sbtMissAddressRegion = {
				.deviceAddress = sbtAddress + baseAlignment * 1,
				.stride = baseAlignment,
				.size = baseAlignment
			};

			sbtHitAddressRegion = {
				.deviceAddress = sbtAddress + baseAlignment * 2,
				.stride = baseAlignment,
				.size = baseAlignment
			};

			uint8_t* sbtBufferData = static_cast<uint8_t*>(device.mapMemory(shaderBindingTableBuffer.memory, 0, sbtBufferSize));
			memcpy(sbtBufferData, handles.data(), handleSize);
			memcpy(sbtBufferData + baseAlignment, handles.data() + handleSize, handleSize);
			memcpy(sbtBufferData + baseAlignment * 2, handles.data() + handleSize * 2, handleSize);
			device.unmapMemory(shaderBindingTableBuffer.memory);
		}(sbtRayGenAddressRegion, sbtMissAddressRegion, sbtHitAddressRegion);

	//Ray-Tracing Output

	auto getImagePipelineBarrier = [](const vk::AccessFlagBits& srcAccessFlags, const vk::AccessFlagBits& dstAccessFlags,
		const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout, const vk::Image& image, uint32_t computeQueueFamily)
		{
			return vk::ImageMemoryBarrier{
			.srcAccessMask = srcAccessFlags,
			.dstAccessMask = dstAccessFlags,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = computeQueueFamily,
			.dstQueueFamilyIndex = computeQueueFamily,
			.image = image,
			.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
					},
			};
		};

	std::vector<vk::CommandBuffer> commandBuffers = device.allocateCommandBuffers({
		.commandPool = commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = imageCount
		});

	#if 1
	for (size_t nn = 0; nn < commandBuffers.size(); nn++) {
		vk::CommandBufferBeginInfo beginInfo = {};
		VK_CHECK_RESULT(commandBuffers[nn].begin(&beginInfo));

		vk::ImageMemoryBarrier imageBarriersToGeneral[2] = {
			getImagePipelineBarrier(
				vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eShaderWrite, 
				vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, 
				renderTargetImage.image, queueId),
		};
		commandBuffers[nn].pipelineBarrier(vk::PipelineStageFlagBits::eRayTracingShaderKHR, 
			vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::DependencyFlagBits::eByRegion, 
			0, nullptr, 0, nullptr, 1, imageBarriersToGeneral);


		//RayTracing
		commandBuffers[nn].bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline);

		std::vector<vk::DescriptorSet> descriptorSets = { rtDescriptorSet };
		commandBuffers[nn].bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, rtPipelineLayout, 0, descriptorSets, nullptr);

		commandBuffers[nn].traceRaysKHR(sbtRayGenAddressRegion, sbtMissAddressRegion, sbtHitAddressRegion, 
			{}, settings.windowWidth, settings.windowHeight, 1, dynamicDispatchLoader);

		vk::ImageMemoryBarrier imageBarriersToTransfer[2] = {
			getImagePipelineBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead, 
				vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, renderTargetImage.image, queueId),
			getImagePipelineBarrier(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferWrite, 
				vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, swapChainImages[nn], queueId)
		};

		commandBuffers[nn].pipelineBarrier(vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlagBits::eByRegion, 0, nullptr,
			0, nullptr, 2, imageBarriersToTransfer);


		vk::ImageSubresourceLayers subresourceLayers = {.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
			};

		vk::ImageCopy imageCopy = {
			.srcSubresource = subresourceLayers, .srcOffset = {0,0,0},
			.dstSubresource = subresourceLayers, .dstOffset = {0,0,0},
			.extent = {.width = settings.windowWidth,
			.height = settings.windowHeight,
			.depth = 1}
			};

		commandBuffers[nn].copyImage(renderTargetImage.image, vk::ImageLayout::eGeneral, swapChainImages[nn],
			vk::ImageLayout::eTransferDstOptimal, 1, &imageCopy);

		vk::ImageMemoryBarrier barrierSwapChainToPresent = getImagePipelineBarrier(
				vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead, 
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR, 
				swapChainImages[nn], queueId);

		commandBuffers[nn].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, 
			vk::PipelineStageFlagBits::eTransfer,vk::DependencyFlagBits::eByRegion, 
			0, nullptr,0, nullptr, 1, &barrierSwapChainToPresent);

		commandBuffers[nn].end();
	}
	#endif

	//CreateFence
	vk::Fence fence = device.createFence({});
	//CreateSemaphore
	vk::Semaphore semaphore = device.createSemaphore({});
	vk::Semaphore semaphore2 = device.createSemaphore({});
	//----------------RenderLoop
	float yAngle = 0.8f;
	while (!glfwWindowShouldClose(window))
	{
		//Essentiallthecameradata
		struct UniformData
		{
		//glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		int samples;
		};
		auto updateUniformBuffer = [&device, &uniformBuffer](UniformData& uniformData)
				{
					void* data = device.mapMemory(uniformBuffer.memory, 0, sizeof(uniformData));
					memcpy(data, &uniformData, sizeof(uniformData));
					device.unmapMemory(uniformBuffer.memory);
				};
		float dist = 10.0f;
		yAngle += 0.02f;
		glm::mat4 ident(1.0f);
		glm::mat4 rotY = glm::rotate(ident, yAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 camZ = glm::vec3(rotY[0][0] * dist, rotY[0][1] * dist, rotY[0][2] * dist);
		UniformData uniformData{};
		uniformData.view = glm::lookAt(camZ, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		uniformData.view = glm::scale(uniformData.view, glm::vec3(2, 2, 2));
		uniformData.samples = sampleNumber;

		//perspective
		uniformData.proj = glm::perspective(glm::radians(60.0f), (float)settings.windowWidth / (float)settings.windowHeight, 0.1f, 1000.0f);

		//ortho
		/*float aspectRatio = (float)settings.windowWidth / (float)settings.windowHeight;
		uniformData.proj = glm::ortho(-2.0f * aspectRatio, 2.0f * aspectRatio, -2.0f, 2.0f, 0.1f, 1000.0f);*/
		updateUniformBuffer(uniformData);

		glfwPollEvents();
		auto swapChainImageIndex = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), semaphore2, {}).value;
		vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eTransfer;
		device.resetFences(fence);
		vk::SubmitInfo submitInfo = {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &semaphore2,
			.pWaitDstStageMask = &waitStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffers[swapChainImageIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &semaphore
			};
		VK_CHECK_RESULT(computePresentQueue.submit(1, &submitInfo, fence));
		VK_CHECK_RESULT(device.waitForFences(1, &fence, true, UINT64_MAX));
		device.resetFences(fence);
		vk::PresentInfoKHR presentInfo = {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &semaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapChain,
			.pImageIndices = &swapChainImageIndex
			};
		VK_CHECK_RESULT(computePresentQueue.presentKHR(presentInfo));
		device.waitIdle();
	}

	//cleanup

	device.destroySemaphore(semaphore);
	device.destroySemaphore(semaphore2);
	device.destroyFence(fence);
	device.destroyPipeline(rtPipeline);
	device.destroyPipelineLayout(rtPipelineLayout);
	//device.destroyDescriptorSetLayout(rtDescriptorSetLayout);
	//device.destroyDescriptorPool(rtDescriptorPool);
	auto destroyBuffer = [&device](const VulkanBuffer& buffer)
		{
			device.destroyBuffer(buffer.buffer);
			device.freeMemory(buffer.memory);
		};
	auto destroyAccelerationStructure = [&device, &destroyBuffer](const VulkanAccelerationStructure& accelerationStructure, 
		vk::DispatchLoaderDynamic& dynamicDispatchLoader)
			{
				device.destroyAccelerationStructureKHR(accelerationStructure.accelerationStructure, nullptr, dynamicDispatchLoader);
				destroyBuffer(accelerationStructure.structureBuffer);
				destroyBuffer(accelerationStructure.scratchBuffer);
				destroyBuffer(accelerationStructure.instancesBuffer);
			};
			destroyAccelerationStructure(topAccelerationStructure, dynamicDispatchLoader);
			destroyAccelerationStructure(bottomAccelerationStructure, dynamicDispatchLoader);
			//destroyBuffer(uniformBuffer, device);
			//destroyBuffer(shaderBindingTableBuffer, device);
			// device.destroyImageView(swapChainImageView); // todo
			device.destroySwapchainKHR(swapChain);
			device.destroyCommandPool(commandPool);
			auto destroyImage = [&device](const VulkanImage& image) {
				device.destroyImageView(image.imageView);
				device.destroyImage(image.image);
				device.freeMemory(image.memory);
				};
			destroyImage(renderTargetImage);
			device.destroy();
			instance.destroySurfaceKHR(surface);
			instance.destroy();
			glfwDestroyWindow(window);
			glfwTerminate();

			return 0;
}