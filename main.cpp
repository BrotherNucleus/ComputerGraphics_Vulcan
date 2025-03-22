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

 vec3 rotate( vec3 pos, float x, float y, float z )
	 {
		 mat3 rotX=mat3( 1.0, 0.0, 0.0, 0.0, cos( x ),-sin( x ), 0.0, sin( x ), cos(x));
		 mat3 rotY=mat3( cos( y ), 0.0, sin( y ), 0.0, 1.0, 0.0,-sin(y), 0.0, cos(y));
		 mat3 rotZ=mat3( cos( z ),-sin( z ), 0.0, sin( z ), cos( z ), 0.0, 0.0, 0.0, 1.0 );
		 return rotX*rotY*rotZ*pos;
	 }
 float hit(vec3 r)
	 {
	 r=rotate(r,0.2,0.5,0.0);
	 vec3 zn=vec3(r.xyz);
	 float rad=0.0;
	 float hit=0.0;
	 float p=10.0;//detail
	 float d=1.0;
	 for(int i=0;i<45;i++)
		 {
		 rad=length(zn);
		 if(rad>2.0)
		 {
			hit=0.5*log(rad)*rad/d;
		 }else{
			 float th=atan(length(zn.xy),zn.z);
			 float phi=atan(zn.y,zn.x);
			 float rado=pow(rad,8.0);
			 d=pow(rad,7.0)*7.0*d+1.0;
			 float sint=sin(th*p);
			 zn.x=rado*sint*cos(phi*p);
			 zn.y=rado*sint*sin(phi*p);
			 zn.z=rado*cos(th*p);
			 zn+=r;
			 }
		 }
	 return hit;
	 }
 vec3 eps=vec3(.12,0.0,0.0);
 void main()
 {
	 vec2 iResolution=vec2(1.0,1.0);
	 vec2 pos=-1.0+2.0*fragUV.xy/iResolution.xy;
	 pos.x *= iResolution.x/iResolution.y;
	 vec3 ro=vec3(pos,-1.2);
	 vec3 la=vec3(0.0,0.0,1.0);
	 vec3 cameraDir=normalize(la-ro);
	 vec3 cameraRight=normalize(cross(cameraDir,vec3(0.0,1.0,0.0)));
	 vec3 cameraUp=normalize(cross(cameraRight,cameraDir));
	 vec3 rd=normalize(cameraDir+vec3(pos,0.0));
	 float t=0.0;
	 float d=200.0;
	 vec3 r;
	 vec3 color=vec3(0.0);
	 for(int i=0;i<100;i++){
		 if( d > .001 )
		 {
			 r=ro + rd * t;
			 d=hit( r );
			 t+=d;
		 }
	 }
	 vec3 n=vec3( hit( r + eps )- hit( r- eps ),
	 hit( r + eps.yxz )- hit( r- eps.yxz ),
	 hit( r + eps.zyx )- hit( r- eps.zyx ) );
	 vec3 mat=vec3( .1, .5, .3 );
	 vec3 light=vec3( .4, .5,-2.0 );
	 vec3 lightCol=vec3(.6, .3, .5);
	 vec3 ldir=normalize( light- r );
	 vec3 diff=dot( ldir, n ) * lightCol * 80.0;
	 color=diff * mat;
	 outColor=vec4( color, 1.0 );
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