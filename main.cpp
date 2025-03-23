#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")
#include <iostream>

#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

#include <shaderc/shaderc.hpp>
#pragma comment(lib, "shadercd.lib")
#pragma comment(lib, "shaderc_utild.lib")
#pragma comment(lib, "shaderc_combinedd.lib")
//vertex shader
std::string vertexShader = R"vertexshader(
	#version 450
	#extension GL_ARB_separate_shader_objects : enable
	out gl_PerVertex{
		vec4 gl_Position;
	};
	layout(location = 0) out vec2 fragUV;
	vec2 positions[4] = vec2[](
		vec2(-1.0, 1.0),
		vec2(-1.0, -1.0),
		vec2(1.0, 1.0),
		vec2(1.0, -1.0)
		);
	vec2 uvs[4] = vec2[](
		vec2(1.0, 1.0),
		vec2(1.0, -1.0),
		vec2(-1.0, 1.0),
		vec2(-1.0, -1.0)
		);

	void main() {
		 gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
		 fragUV = uvs[gl_VertexIndex] * 0.5 + 0.5; // 0->1;
	 }
	 )vertexshader";

std::string fragmentShader = R"fragmentShader(
 #version 450
 #extension GL_ARB_separate_shader_objects : enable

 layout(location=0) in vec2 fragUV;
 layout(location=0) out vec4 outColor;

mat3 rotate( float x, float y, float z )
	 {
		 mat3 rotX=mat3( 1.0, 0.0, 0.0, 0.0, cos( x ),-sin( x ), 0.0, sin( x ), cos(x));
		 mat3 rotY=mat3( cos( y ), 0.0, sin( y ), 0.0, 1.0, 0.0,-sin(y), 0.0, cos(y));
		 mat3 rotZ=mat3( cos( z ),-sin( z ), 0.0, sin( z ), cos( z ), 0.0, 0.0, 0.0, 1.0 );
		 return rotX*rotY*rotZ;
	 }

vec3 trianglehit( vec3 orig, vec3 dir, vec3 a, vec3 b, vec3 c)
{
	vec3 nohit = vec3(0.1, 0.1, 0.1);
	vec3 hitback = vec3(0.4, 0.2, 0.2);
	
	float eps = 0.0000001;
	vec3 ab = b - a;
	vec3 ac = c - a;

	vec3 n=cross(dir, ac);
	float det=dot(ab,n);

	vec3 ao = orig-a;
	float u=dot(ao,n)/det;

	if(u<0.0 || u>1.0) {return nohit;}

	vec3 e = cross(ao, ab);
	float v = dot(dir, e)/det;
	
	if(v<0.0 || (u+v)>1.0) {return nohit;}

	if (det<=eps){ return hitback; }

	float t = dot(ac, e)/det;

	vec3 hitpos = orig + dir * t;
	vec3 lightpos = vec3(1.0, 0.2, -3.0);
	vec3 dir2 = normalize(hitpos - lightpos);
	vec3 norm = normalize( cross( ab,ac ) );
	float illum = clamp( dot( norm, dir2 ), 0.0, 1.0);
	return vec3(1.0, 1.0, 1.0) * illum;
}

void main() {
	vec3 a = vec3(0.0, 0.5, 0.0);
	vec3 b = vec3(-2.0, -0.5, 0.0);
	vec3 c = vec3(1.0, -0.5, 0.0);

	mat3 rot = rotate(0.0, 1.14, 0.0);
	a = rot*a;
	b = rot*b;
	c = rot*c;

	float near=0.2;
	vec3 p=vec3( fragUV*2.0 - 1.0, near);

	vec3 camera = vec3(0.0, 0.0, 5.0);
	vec3 dir = p-camera;
	dir = normalize(dir);

	vec3 col = trianglehit(camera, dir, a, b, c);
	outColor = vec4(col, 1.0);
}
 )fragmentShader";

int main()
{
	const uint32_t width = 640, height = 480;
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	auto window = glfwCreateWindow(width, height, "Vulkan Triangle", nullptr, nullptr);
	vk::ApplicationInfo appInfo("Vulkan Triangle", 0, nullptr, 0, VK_API_VERSION_1_4);
	auto glfwExtensionCount = 0u;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + glfwExtensionCount);
	const vk::ApplicationInfo applicationInfo("Hello world", 0, nullptr, 0, VK_API_VERSION_1_4);
	const auto instance = vk::createInstanceUnique(vk::InstanceCreateInfo({}, &applicationInfo, 0, nullptr, static_cast<uint32_t>(glfwExtensionsVector.size()), glfwExtensionsVector.data()));
	const auto physicalDevice = instance->enumeratePhysicalDevices()[0];

	VkSurfaceKHR surfaceTMP;
	VkResult err = glfwCreateWindowSurface(*instance, window, nullptr, &surfaceTMP);
	vk::UniqueSurfaceKHR surface(surfaceTMP, *instance);

	size_t presentQueueFamilyIndex = 0u;
	float queuePriority = 0.0f;
	auto queueCreateInfos = vk::DeviceQueueCreateInfo{ vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(0), 1, &queuePriority };

	const std::vector<const char*> deviceExtenstions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::UniqueDevice device = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), 1, &queueCreateInfos, 0u, nullptr, static_cast<uint32_t>(deviceExtenstions.size()), deviceExtenstions.data()));

	uint32_t imageCount = 2;
	auto format = vk::Format::eB8G8R8A8Unorm;

	vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, surface.get(), imageCount, format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D{width,height}, 1, 
		vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0u, static_cast<uint32_t*>(nullptr), vk::SurfaceTransformFlagBitsKHR::eIdentity,
		vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eFifo, true, nullptr);
	auto swapChain = device->createSwapchainKHRUnique(swapChainCreateInfo);

	std::vector<vk::Image>swapChainImages = device->getSwapchainImagesKHR(swapChain.get());
	std::vector<vk::UniqueImageView>imageViews;
	imageViews.reserve(swapChainImages.size());
	for (auto image : swapChainImages) {
		{
			{
				vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image,
					vk::ImageViewType::e2D, format, vk::ComponentMapping{ vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB,vk::ComponentSwizzle::eA },
					vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor,0,1,0,1});
				imageViews.push_back(device->createImageViewUnique(imageViewCreateInfo));
			}
		}
	}

	auto compileShader = [&](const std::string& srcGLSL, shaderc_shader_kind shaderKind)
			{
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;
				options.SetOptimizationLevel(shaderc_optimization_level_performance);
				shaderc::SpvCompilationResult shaderModule = compiler.CompileGlslToSpv(srcGLSL, shaderKind, "shader", options);
				if (shaderModule.GetCompilationStatus()!=shaderc_compilation_status_success) {
						{
							{
								std::cerr << shaderModule.GetErrorMessage();
							}
						}
				}
				auto shaderCode = std::vector<uint32_t>{shaderModule.cbegin(),shaderModule.cend()};
				auto shaderSize = std::distance(shaderCode.begin(), shaderCode.end());
				auto shaderCreateInfo = vk::ShaderModuleCreateInfo{ {},shaderSize *sizeof(uint32_t),shaderCode.data()};
				return device->createShaderModuleUnique(shaderCreateInfo);
			};

	auto vertexShaderModule = compileShader(vertexShader,shaderc_glsl_vertex_shader);
	auto fragmentShaderModule = compileShader(fragmentShader,shaderc_glsl_fragment_shader);
	auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},vk::ShaderStageFlagBits::eVertex, *vertexShaderModule,"main" };
	auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},vk::ShaderStageFlagBits::eFragment,*fragmentShaderModule,"main"};
	auto pipelineShaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{vertShaderStageInfo,fragShaderStageInfo};
	auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{ {},0u,nullptr,0u,nullptr};
	auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{ {},vk::PrimitiveTopology::eTriangleStrip,false};
	auto viewport = vk::Viewport{0.0f,0.0f,static_cast<float>(width),static_cast<float>(height),0.0f,1.0f};
	auto scissor = vk::Rect2D{ {0,0},vk::Extent2D{width,height} };

	auto viewportState = vk::PipelineViewportStateCreateInfo{ {},1,&viewport,1,&scissor};
	auto rasterizer = vk::PipelineRasterizationStateCreateInfo{ {},/*depthClamp*/false,
		/*rasterizeDiscard*/false,vk::PolygonMode::eFill,{},
		/*frontFace*/vk::FrontFace::eCounterClockwise,{},{},{},{},1.0f};
	auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{ {},/*srcCol*/vk::BlendFactor::eOne,
		/*dstCol*/vk::BlendFactor::eZero,/*colBlend*/vk::BlendOp::eAdd,
		/*srcAlpha*/vk::BlendFactor::eOne,/*dstAlpha*/vk::BlendFactor::eZero,
		/*alphaBlend*/vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
	auto colorBlending = vk::PipelineColorBlendStateCreateInfo{ {},false,vk::LogicOp::eCopy,1,&colorBlendAttachment};
	auto pipelineLayout = device->createPipelineLayoutUnique({}, nullptr);
	auto colorAttachment = vk::AttachmentDescription{ {},format,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,{},{},{},vk::ImageLayout::ePresentSrcKHR};
	auto colourAttachmentRef = vk::AttachmentReference{0,vk::ImageLayout::eColorAttachmentOptimal};
	auto subpass = vk::SubpassDescription{ {},vk::PipelineBindPoint::eGraphics,0,nullptr,1,&colourAttachmentRef};
	auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
	auto imageAvailableSemaphore = device->createSemaphoreUnique(semaphoreCreateInfo);
	auto renderFinishedSemaphore = device->createSemaphoreUnique(semaphoreCreateInfo);
	auto subpassDependency = vk::SubpassDependency{VK_SUBPASS_EXTERNAL,0,vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::PipelineStageFlagBits::eColorAttachmentOutput,
		{},vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite};
	auto renderPass = device->createRenderPassUnique(
		vk::RenderPassCreateInfo{ {},1,&colorAttachment,1,&subpass,1,&subpassDependency});
	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo{ {},2,pipelineShaderStages.data(),
		&vertexInputInfo,&inputAssembly,nullptr,&viewportState,&rasterizer,nullptr,
		nullptr,&colorBlending,nullptr,*pipelineLayout,*renderPass,0};
	auto pipeline = device->createGraphicsPipelineUnique({}, pipelineCreateInfo).value;
	auto framebuffers = std::vector<vk::UniqueFramebuffer>(imageCount);
	for (size_t i = 0;i < imageViews.size();i++) {
		{
			{
				framebuffers[i] = device->createFramebufferUnique(vk::FramebufferCreateInfo{{},*renderPass,1,&(*imageViews[i]),width,height,1});
			}
		}
	}
	auto commandPoolUnique = device->createCommandPoolUnique({{},static_cast<uint32_t>(presentQueueFamilyIndex)});

	std::vector<vk::UniqueCommandBuffer>commandBuffers =
		device->allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo(commandPoolUnique.get(), vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(framebuffers.size())));
	auto deviceQueue = device->getQueue(static_cast<uint32_t>(presentQueueFamilyIndex), 0);
	for (size_t i = 0;i < commandBuffers.size();i++) {
		{
			{
				auto beginInfo = vk::CommandBufferBeginInfo{};
				commandBuffers[i]->begin(beginInfo);
				vk::ClearValue clearValues{};
				auto renderPassBeginInfo = vk::RenderPassBeginInfo{ renderPass.get(),framebuffers[i].get(),vk::Rect2D{{0,0},vk::Extent2D{width,height}},1,&clearValues};
				commandBuffers[i]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
				commandBuffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
				commandBuffers[i]->draw(4, 1, 0, 0);
				commandBuffers[i]->endRenderPass();
				commandBuffers[i]->end();
			}
		}
	}
	while (!glfwWindowShouldClose(window))
	{
		{
			{
				glfwPollEvents();
				auto imageIndex = device->acquireNextImageKHR(swapChain.get(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore.get(), {});
				vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
				auto submitInfo = vk::SubmitInfo{1,&imageAvailableSemaphore.get(),& waitStageMask,1,&commandBuffers[imageIndex.value].get(),1,&renderFinishedSemaphore.get()};
				deviceQueue.submit(submitInfo, {});
				auto presentInfo = vk::PresentInfoKHR{1,&renderFinishedSemaphore.get(),1,&swapChain.get(),&imageIndex.value};
				auto result = deviceQueue.presentKHR(presentInfo);
				device->waitIdle();
			}
		}
	}
 }