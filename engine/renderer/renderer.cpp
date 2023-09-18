#include <renderer.hpp>
#include <utils.hpp>

#include <optional>
#include <stdint.h>
#include <tuple>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstring>		// strcmp
#include <map>			// std::multimap
#include <set>			// std::set doesn't allow duplicates

#include <thread>
#include <chrono>

#include <limits>		// std::numeric_limits
#include <algorithm>	// std::clamp

#include <fstream>		// for reading the shaders

#include <glm/glm.hpp>

#include <debugUtils.hpp>
#include <wrappers.hpp>

Renderer::Renderer()
{
	initWindow();
	initVulkan();

	// initial cell state pls move this somewhere else ffs
	for (int y = 0; y < GRID_SIZE_Y; ++y) {
		ssbo.cell_state[y * GRID_SIZE_X + 1].value = 1;
		ssbo.cell_state[y * GRID_SIZE_X + GRID_SIZE_X / 2].value = 1;
		ssbo.cell_state[y * GRID_SIZE_X + GRID_SIZE_X - 2].value = 1;
	}

	// for (int y = 0; y < GRID_SIZE_X; ++y) {
	// 	for (int x = 0; x < GRID_SIZE_Y; ++x) {
	// 		ssbo.cell_state[GRID_SIZE_X * y + x].value = 0;
	// 	}
	// }
}

void Renderer::run()
{
	mainLoop();
	cleanUp();
}

// ???????????????????????????????????
// void (*Renderer::updateFunc) (std::array<Cell, GRID_SIZE_X * GRID_SIZE_Y>&) = nullptr;

void Renderer::initWindow()
{
	glfwInit();
	glfwSetErrorCallback(glfwErrorCallback);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "HAHAHA", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int x, int y){
		Renderer* app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(win));
		app->framebufferResized = true;
	});
}

void Renderer::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();

	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createStorageBuffer();
	createCommandBuffers();
	createDescriptorPool();
	createDescriptorSet();

	createSyncObjects();
}

void Renderer::mainLoop()
{
	auto start_time = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto delta = now - start_time;
		auto end = now + std::chrono::milliseconds(MIN_FRAME_TIME);

		// if (delta >= std::chrono::seconds(1)) {
		// 	start_time = now;
		//	std::cout << "Frame Time: ";
		// }

		updateCells();
		glfwPollEvents();
		drawFrame();

		std::this_thread::sleep_until(end);
	}

	device.waitIdle();
}

void Renderer::drawFrame()
{
	// the * operator on vk::raii::<something> returns vk::<something> & (remember it's a reference)
	while (device.waitForFences(*inFlightFences[currentFrame], VK_TRUE, UINT32_MAX) == vk::Result::eTimeout)
		;

	auto [result, imageIndex] = SwapchainNextImageWrapper(swapchain, UINT64_MAX, *imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		recreateSwapchain();
		return;
	} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("[Swapchain]: Failed to acquire a swapchain image.");
	}
	
	// delay fence reset to prevent deadlock if swapchain needs to be recreated
	device.resetFences(*inFlightFences[currentFrame]);

	commandBuffers[currentFrame].reset();
	recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	vk::Semaphore waitSemaphores[] = { *imageAvailableSemaphores[currentFrame] };
	vk::Semaphore signalSemaphores[] = { *renderFinishedSemaphores[currentFrame] };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::SubmitInfo submitInfo(waitSemaphores, waitStages, *commandBuffers[currentFrame], signalSemaphores);
	graphicsQueue.submit(submitInfo, *inFlightFences[currentFrame]);

	vk::SwapchainKHR swapchains[] = { *swapchain };
	vk::PresentInfoKHR presentInfo(signalSemaphores, swapchains, imageIndex);
	
  	// throws Poco::NotFoundException, apparently a driver issue
	result = QueuePresentWrapper(instance, presentQueue, presentInfo);
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	} else if(result != vk::Result::eSuccess){
		throw std::runtime_error("[Queue]: Failed to present Graphics Queue!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::cleanUp()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Renderer::recreateSwapchain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while((width == 0 || height == 0)) {
		if (glfwWindowShouldClose(window)) {
			return;
		}
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	// The reason we're doing device wait idle instead of just waiting on a fence is we don't know what state the gpu is in 
	device.waitIdle();	

	swapchainFramebuffers.clear();
	swapchainImageViews.clear();
	swapchain.clear();

	Renderer::createSwapchain();
	Renderer::createFramebuffers();
}

void Renderer::createInstance()
{
	vk::ApplicationInfo appInfo{
		"TriangleTest",						// pApplicationName
		VK_MAKE_API_VERSION(0, 1, 0, 0),	// applicationVersion
		"NoRenderer",							// pRendererName
		VK_MAKE_API_VERSION(0, 1, 0, 0),	// RendererVersion
		VK_API_VERSION_1_0					// apiVersion
	};

	std::vector<const char*> extensions = getRequiredInstanceExtensions();
	
	vk::DebugUtilsMessengerCreateInfoEXT debugInstaceCreateInfo;
	if (enableValidationLayers)
	{
		populateDebugMessengerCreateInfo(debugInstaceCreateInfo);
	}

	vk::InstanceCreateInfo createInfo{
		vk::InstanceCreateFlags{},									// flags;
		&appInfo,													// pApplicationInfo;
		static_cast<uint32_t>(enableValidationLayers ? ValidationLayers.size() : 0),
																	// enabledLayerCount;
		enableValidationLayers ? ValidationLayers.data() : nullptr,	// ppEnabledLayerNames;
		static_cast<uint32_t>(extensions.size()),					// enabledExtensionCount;
		extensions.data()											// ppEnabledExtensionNames;
	};

	// CreateInstance
	instance = vk::raii::Instance(context, createInfo);

	// Check if all Validation Layer are supported
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("[VK_Instance]: Validation Layers requested but not available!");
	}
}

void Renderer::pickPhysicalDevice()
{

	vk::raii::PhysicalDevices devices(instance);
	if (devices.size() == 0)
	{
		throw std::runtime_error("[VK_Instance]: No Graphics Devices with Vulkan support!");
	}

	std::multimap<int, vk::raii::PhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// the candidate with the highest score is selected
	if (candidates.rbegin()->first > 0)
	{
		physicalDevice = candidates.rbegin()->second;
	} 
	else if (candidates.rbegin()->first == -1)
	{
		throw std::runtime_error("[VK_Instance]: No Supported Graphics Devices with required Queue Families!");
	}
	else if (candidates.rbegin()->first == -2)
	{
		throw std::runtime_error("[VK_Instance]: No Supported Graphics Devices with required Device Extensions!");
	}
	else if (candidates.rbegin()->first == -3)
	{
		throw std::runtime_error("[VK_Instance]: No Supported Graphics Devices with required Swap Chain Support!");
	}
	else
	{
		throw std::runtime_error("[VK_Instance]: No Supported Graphics Devices found!");
	}
}

void Renderer::createLogicalDevice()
{
	QueueFamilyIndices indices = queryQueueFamilyIndices(physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	// sets don't allow duplicates
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{
			vk::DeviceQueueCreateFlags{},				// flags;
			queueFamily,								// queueFamilyIndex;
			1,											// queueCount;
			&queuePriority								// pQueuePriorities;
		};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures;

	vk::DeviceCreateInfo deviceCreateInfo{
		vk::DeviceCreateFlags{},									// flags;
		static_cast<uint32_t>(queueCreateInfos.size()),				// queueCreateInfoCount;
		queueCreateInfos.data(),									// pQueueCreateInfos;
		static_cast<uint32_t>(enableValidationLayers ? ValidationLayers.size() : 0),
																	// enabledLayerCount;
		enableValidationLayers ? ValidationLayers.data() : nullptr,	// ppEnabledLayerNames;
		static_cast<uint32_t>(DeviceExtensions.size()),				// enabledExtensionCount;
		DeviceExtensions.data(),									// ppEnabledExtensionNames;
		&deviceFeatures												// pEnabledFeatures;
	};

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	
	graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
	presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void Renderer::createSurface()
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("[VK_Instance]: Failed to create a Surface.");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
}

void Renderer::createSwapchain()
{
	SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
	vk::PresentModeKHR presentMode = choostSwapPresentMode(swapchainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

	// minImageCount may mean waiting on the drivers thus +1
	uint32_t minImageCount = swapchainSupport.capabilities.minImageCount + 1;
	// If minImageCount + 1 is more than maxImageCount
	if (minImageCount > swapchainSupport.capabilities.maxImageCount && swapchainSupport.capabilities.maxImageCount > 0)
	{
		minImageCount = swapchainSupport.capabilities.maxImageCount;
	}

	QueueFamilyIndices indices = queryQueueFamilyIndices(physicalDevice);
	uint32_t queueFamilyIndices[2] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};
	bool exclusive = indices.graphicsFamily == indices.presentFamily;

	vk::SwapchainCreateInfoKHR createInfo{
		vk::SwapchainCreateFlagsKHR{},					// flags
		*surface,										// surface
		minImageCount,									// minImageCount
		surfaceFormat.format,							// imageFormat
		surfaceFormat.colorSpace,						// imageColorSpace
		extent,											// imageExtent
		1,												// imageArrayLayers
		vk::ImageUsageFlagBits::eColorAttachment,		// imageUsage
		exclusive? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
														// imageSharingMode
		static_cast<uint32_t>(exclusive ? 0 : 2),		// queueFamilyIndexCount
		exclusive? nullptr : queueFamilyIndices,		// pQueueFamilyIndices
		swapchainSupport.capabilities.currentTransform,	// preTransform
		vk::CompositeAlphaFlagBitsKHR::eOpaque,			// compositeAlpha
		presentMode,									// presentMode
		true,											// clipped
		// *swapchain										// oldSwapchain
		vk::SwapchainKHR{}								// oldSwapchain
	};

	swapchain = vk::raii::SwapchainKHR(device, createInfo);

	swapchainImages = swapchain.getImages();

	swapchainImageFormat = surfaceFormat.format;
	swapchainImageExtent = extent;

	//Create Image Views for Swapchain Images
	swapchainImageViews.reserve(swapchainImages.size());
	for (uint32_t i = 0; i < swapchainImages.size(); ++i)
	{
		vk::ImageViewCreateInfo createInfo{
			vk::ImageViewCreateFlags{},					// flags;
			swapchainImages[i],							// image;
			vk::ImageViewType::e2D,						// viewType;
			swapchainImageFormat,						// format;
			vk::ComponentMapping(						// components;
				vk::ComponentSwizzle::eIdentity,	// r
				vk::ComponentSwizzle::eIdentity,	// g
				vk::ComponentSwizzle::eIdentity,	// b
				vk::ComponentSwizzle::eIdentity		// a
			),
			vk::ImageSubresourceRange(					// subresourceRange
				vk::ImageAspectFlagBits::eColor,	// aspectMask;
				0,									// baseMipLevel;
				1,									// levelCount;
				0,									// baseArrayLayer;
				1									// layerCount;
			)
		};
		
		swapchainImageViews.emplace_back( device, createInfo );
	}

}

void Renderer::createRenderPass()
{
	vk::AttachmentDescription colorAttachment{
		vk::AttachmentDescriptionFlags{},	// flags
		swapchainImageFormat,				// format
		vk::SampleCountFlagBits::e1,		// samples
		vk::AttachmentLoadOp::eClear,		// loadOp
		vk::AttachmentStoreOp::eStore,		// storeOp
		vk::AttachmentLoadOp::eClear,		// stencilLoadOp
		vk::AttachmentStoreOp::eDontCare,	// stencilStoreOp
		vk::ImageLayout::eUndefined,		// initialLayout
		vk::ImageLayout::ePresentSrcKHR		// finalLayout
	};

	vk::AttachmentReference colorAttachmentReference{
		0,											// attachment
		vk::ImageLayout::eColorAttachmentOptimal	// layout
	};

	vk::SubpassDescription subpass{
		vk::SubpassDescriptionFlags{},		// flags
		vk::PipelineBindPoint::eGraphics,	// pipelineBindPoint
		0,									// inputAttachmentCount
		0,									// pInputAttachments
		1,									// colorAttachmentCount
		&colorAttachmentReference,			// pColorAttachments
		nullptr,							// pResolveAttachments
		nullptr,							// pDepthStencilAttachment
		0,									// preserveAttachmentCount
		nullptr,							// pPreserveAttachments
	};

	vk::SubpassDependency subpassDependency{
		VK_SUBPASS_EXTERNAL,								// srcSubpass
		0,													// dstSubpass
		vk::PipelineStageFlagBits::eColorAttachmentOutput,	// srcStageMask
		vk::PipelineStageFlagBits::eColorAttachmentOutput,	// dstStageMask
		vk::AccessFlagBits::eNone,							// srcAccessMask
		vk::AccessFlagBits::eColorAttachmentWrite,			// dstAccessMask
		vk::DependencyFlags{}								// dependencyFlags
	};

	vk::RenderPassCreateInfo renderPassInfo{
		vk::RenderPassCreateFlagBits{},				// flags
		1,											// attachmentCount
		&colorAttachment,							// pAttachments
		1,											// subpassCount
		&subpass,									// pSubpasses
		1,											// dependencyCount
		&subpassDependency							// pDependencies
	};

	renderPass = vk::raii::RenderPass(device, renderPassInfo);
}

void Renderer::createDescriptorSetLayout() {

	vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	vk::DescriptorSetLayoutBinding ssboLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex);

	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings = {
		uboLayoutBinding,
		ssboLayoutBinding
	};

	descriptorSetLayout = vk::raii::DescriptorSetLayout(
		device, vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags{},
			layoutBindings
		)
	);
}

void Renderer::createGraphicsPipeline()
{
	std::vector<char> vertexShaderCode = readFile(utils::getExecutableDir() / "res/shaders/testv.spv");
	std::vector<char> fragmentShaderCode = readFile(utils::getExecutableDir() / "res/shaders/testf.spv");

	vk::raii::ShaderModule vertShaderModule = createShaderModule(vertexShaderCode);
	vk::raii::ShaderModule fragShaderModule = createShaderModule(fragmentShaderCode);

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		vk::PipelineShaderStageCreateFlags{},					// flags
		vk::ShaderStageFlagBits::eVertex,						// stage
		*vertShaderModule,										// module
		"main",													// pName -> Entrypoint
		nullptr													// pSpecializationInfo
	};

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		vk::PipelineShaderStageCreateFlags{},					// flags
		vk::ShaderStageFlagBits::eFragment,						// stage
		*fragShaderModule,										// module
		"main",													// pName -> Entrypoint
		nullptr													// pSpecializationInfo
	};

	vk::PipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescription = Vertex::getAttributeDescription();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
		vk::PipelineVertexInputStateCreateFlags{},				// flags;
		1,														// vertexBindingDescriptionCount;
		&bindingDescription,									// pVertexBindingDescriptions;
		attributeDescription.size(),							// vertexAttributeDescriptionCount;
		attributeDescription.data()								// pVertexAttributeDescriptions;
	};

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo {
		vk::PipelineInputAssemblyStateCreateFlags{},					// flags
		vk::PrimitiveTopology::eTriangleList,							// topology
		VK_FALSE														// primitiveRestartEnable
	};

	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo {
		vk::PipelineDynamicStateCreateFlags{},					// flags
		static_cast<uint32_t>(dynamicStates.size()),			// dynamicStateCount
		dynamicStates.data()									// pDynamicStates
	};

	vk::PipelineViewportStateCreateInfo viewportStateInfo {
		vk::PipelineViewportStateCreateFlags{},					// flags
		1,														// viewportCount
		nullptr,												// pViewports
		1,														// scissorCount
		nullptr													// pScissors
	};
	// We'll define the viewports and scissors at draw time

	vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo {
		vk::PipelineRasterizationStateCreateFlags{},				// flags
		VK_FALSE,													// depthClampEnable
		VK_FALSE,													// rasterizerDiscardEnable
		vk::PolygonMode::eFill,										// polygonMode
		vk::CullModeFlagBits::eBack,								// cullMode
		vk::FrontFace::eClockwise,									// frontFace
		VK_FALSE,													// depthBiasEnable
		0.0f,														// depthBiasConstantFactor
		0.0f,														// depthBiasClamp
		0.0f,														// depthBiasSlopeFactor
		1.0f														// lineWidth
	};

	vk::PipelineMultisampleStateCreateInfo multisampleInfo{
		vk::PipelineMultisampleStateCreateFlags{},					// flags
		vk::SampleCountFlagBits::e1,								// rasterizationSamples
		VK_FALSE,													// sampleShadingEnable
		1.0f,														// minSampleShading
		nullptr,													// pSampleMask
		VK_FALSE,													// alphaToCoverageEnable
		VK_FALSE													// alphaToOneEnable
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		VK_FALSE,								// blendEnable
		vk::BlendFactor::eOne,					// srcColorBlendFactor
		vk::BlendFactor::eZero,					// dstColorBlendFactor
		vk::BlendOp::eAdd,						// colorBlendOp
		vk::BlendFactor::eOne,					// srcAlphaBlendFactor
		vk::BlendFactor::eZero,					// dstAlphaBlendFactor
		vk::BlendOp::eAdd,						// alphaBlendOp
		vk::ColorComponentFlagBits::eR |				// colorWriteMask
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA
	};
	// I'm not sure what the colorWriteMask should be so check everything once

	vk::PipelineColorBlendStateCreateInfo colorBlendInfo{
		vk::PipelineColorBlendStateCreateFlags{},					// flags
		VK_FALSE,				                                    // logicOpEnable
		vk::LogicOp::eCopy,		                                    // logicOp
		1,					                                        // attachmentCount
		&colorBlendAttachment,										// pAttachments
		{															// blendConstants[4]
			0.0f,
			0.0f,
			0.0f,
			0.0f
		}
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		vk::PipelineLayoutCreateFlags{},				//flags
		*descriptorSetLayout,
		nullptr
	);
	
	pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

	vk::GraphicsPipelineCreateInfo pipelineInfo{
		vk::PipelineCreateFlags{},							//flags
		2,													//stageCount
		shaderStages,										//pStages
		&vertexInputInfo,									//pVertexInputState
		&inputAssemblyInfo,									//pInputAssemblyState
		nullptr,											//pTessellationState
		&viewportStateInfo,									//pViewportState
		&rasterizationStateInfo,							//pRasterizationState
		&multisampleInfo,									//pMultisampleState
		nullptr,											//pDepthStencilState
		&colorBlendInfo,									//pColorBlendState
		&dynamicStateCreateInfo,							//pDynamicState
		*pipelineLayout,									//layout
		*renderPass,										//renderPass
		0,													//subpass
		VK_NULL_HANDLE,                                     //basePipelineHandle
		-1													//basePipelineIndex
	};

	graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineInfo);
}

void Renderer::createFramebuffers()
{
	swapchainFramebuffers.reserve(swapchainImages.size());

	for (size_t i = 0; i < swapchainImageViews.size(); ++i)
	{
		vk::ImageView attachments[] = { *swapchainImageViews[i] };

		vk::FramebufferCreateInfo framebufferInfo{
			vk::FramebufferCreateFlags{},				//flags
			*renderPass,								//renderPass
			1,											//attachmentCount
			attachments,								//pAttachments
			swapchainImageExtent.width,					//width
			swapchainImageExtent.height,				//height
			1											//layers
		};

		// swapchainFramebuffers[i] = vk::raii::Framebuffer( device, framebufferInfo );
		swapchainFramebuffers.emplace_back( device, framebufferInfo );
	}
}

void Renderer::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = queryQueueFamilyIndices(physicalDevice);

	vk::CommandPoolCreateInfo commandPoolCreateInfo{
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,	//flags
		queueFamilyIndices.graphicsFamily.value()			//queueFamilyIndex
	};

	commandPool = vk::raii::CommandPool(device, commandPoolCreateInfo);
}

[[nodiscard]] auto Renderer::createBuffer(
	vk::DeviceSize _size, 
	vk::BufferUsageFlags _usage, 
	vk::MemoryPropertyFlags _propertyFlags
) -> std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> {
	vk::BufferCreateInfo bufferInfo{
		vk::BufferCreateFlags{},				// flags_
		_size, 									// size_
		_usage,									// usage_
		vk::SharingMode::eExclusive,			// sharingMode_
		0, 										// queueFamilyIndexCount_
		nullptr,								// pQueueFamilyIndices_
		nullptr,								// pNext_
	};

	auto buffer = vk::raii::Buffer(device, bufferInfo);

	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
	
	uint32_t memoryTypeIndex = 0;
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		if (
			(memRequirements.memoryTypeBits & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & _propertyFlags)
		) {
			memoryTypeIndex = i;
		}
	}

	vk::MemoryAllocateInfo allocInfo{
		memRequirements.size,
		memoryTypeIndex,
		nullptr
	};

	auto bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
	buffer.bindMemory(*bufferMemory, 0);

	return std::make_pair( std::move(buffer), std::move(bufferMemory) );
} 

void Renderer::copyBuffer(vk::raii::Buffer& _src, vk::raii::Buffer& _dst, vk::DeviceSize _size)
{
	vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1);
	vk::raii::CommandBuffer copyCommandBuffer = std::move( vk::raii::CommandBuffers(device, allocInfo).front() );

	copyCommandBuffer.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
	copyCommandBuffer.copyBuffer(*_src, *_dst, std::array<vk::BufferCopy, 1>{vk::BufferCopy(0, 0, _size)});
	copyCommandBuffer.end();

	graphicsQueue.submit(std::array<vk::SubmitInfo, 1>{vk::SubmitInfo( nullptr, nullptr, *copyCommandBuffer, nullptr )});
	graphicsQueue.waitIdle();

	copyCommandBuffer.clear();
}

void Renderer::createVertexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	vk::raii::Buffer stagingBuffer(nullptr);
	vk::raii::DeviceMemory stagingBufferMemory(nullptr);
	std::tie(stagingBuffer, stagingBufferMemory) = createBuffer(bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(data, vertices.data(), (size_t)bufferSize);

	std::tie(vertexBuffer, vertexBufferMemory) = createBuffer(bufferSize,
	 			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
	 			vk::MemoryPropertyFlagBits::eDeviceLocal);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void Renderer::createIndexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(uint16_t) * indices.size();

	vk::raii::Buffer stagingBuffer(nullptr);
	vk::raii::DeviceMemory stagingBufferMemory(nullptr);
	std::tie(stagingBuffer, stagingBufferMemory) = createBuffer(bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(data, indices.data(), (size_t)bufferSize);

	std::tie(indexBuffer, indexBufferMemory) = createBuffer(bufferSize,
	 			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
	 			vk::MemoryPropertyFlagBits::eDeviceLocal);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

void Renderer::createUniformBuffers()
{
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	std::tie(uniformBuffer, uniformBuffersMemory) = createBuffer(bufferSize, 
		vk::BufferUsageFlagBits::eUniformBuffer, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	void* uniformBufferMapped = uniformBuffersMemory.mapMemory(0, bufferSize);
	memcpy(uniformBufferMapped, &ubo, bufferSize);
}

void Renderer::createStorageBuffer()
{
	vk::DeviceSize bufferSize = sizeof(StorageBufferObject);

	std::tie(storageBuffer, storageBufferMemory) = createBuffer(bufferSize, 
		vk::BufferUsageFlagBits::eStorageBuffer, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	storageBufferWriteLoc = storageBufferMemory.mapMemory(0, bufferSize);

	memcpy(storageBufferWriteLoc, &ssbo, bufferSize);
}

void Renderer::setCellUpdate(void f(std::array<Cell, GRID_SIZE_X * GRID_SIZE_Y>&))
{
	updateFunc = f;
}

void Renderer::updateCells()
{
	vk::DeviceSize bufferSize = sizeof(StorageBufferObject);

	updateFunc(ssbo.cell_state);
	
	memcpy(storageBufferWriteLoc, &ssbo, bufferSize);
	cell_state_in = ssbo.cell_state;
}

void Renderer::createDescriptorPool()
{
	auto poolSizes = std::vector<vk::DescriptorPoolSize> {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1),
	};

	vk::DescriptorPoolCreateInfo poolInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		1,
		poolSizes
	);
	descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

void Renderer::createDescriptorSet()
{
	auto allocInfo = vk::DescriptorSetAllocateInfo(*descriptorPool, *descriptorSetLayout);

	vk::raii::DescriptorSets descriptorSets(device, allocInfo);
	descriptorSet = vk::raii::DescriptorSet( std::move(descriptorSets.front()) );
	
	vk::DescriptorBufferInfo uniformBufferInfo(*uniformBuffer, 0, sizeof(UniformBufferObject));
	vk::DescriptorBufferInfo storageBufferInfo(*storageBuffer, 0, sizeof(StorageBufferObject));

	vk::WriteDescriptorSet descriptorWrite(*descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, uniformBufferInfo, nullptr, nullptr);
	device.updateDescriptorSets(descriptorWrite, nullptr);

	descriptorWrite = vk::WriteDescriptorSet(*descriptorSet, 1, 0, vk::DescriptorType::eStorageBuffer, nullptr, storageBufferInfo, nullptr, nullptr);
	device.updateDescriptorSets(descriptorWrite, nullptr);
}

void Renderer::createCommandBuffers()
{
	commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
		*commandPool,									//commandPool
		vk::CommandBufferLevel::ePrimary,				//level
		(uint32_t) MAX_FRAMES_IN_FLIGHT					//commandBufferCount
	};

	commandBuffers = vk::raii::CommandBuffers(device, commandBufferAllocateInfo);
}

void Renderer::createSyncObjects()
{
	imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

	vk::SemaphoreCreateInfo semaphoreCreateInfo;

	vk::FenceCreateInfo fenceCreateInfo{
		vk::FenceCreateFlagBits::eSignaled				//flags	-> first frame waits for a signaled fence
	};

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		imageAvailableSemaphores.emplace_back( device, semaphoreCreateInfo );
		renderFinishedSemaphores.emplace_back( device, semaphoreCreateInfo );
		inFlightFences.emplace_back( device, fenceCreateInfo );	
	}
}

void Renderer::recordCommandBuffer(vk::raii::CommandBuffer& _commandBuffer, uint32_t _imageIndex)
{
	_commandBuffer.begin( vk::CommandBufferBeginInfo( vk::CommandBufferUsageFlags() ) );
	
	vk::ClearValue clearValue;
	clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

	vk::RenderPassBeginInfo renderPassBeginInfo{
		*renderPass,									//renderPass
		*swapchainFramebuffers[_imageIndex],			//framebuffer
		vk::Rect2D {									//renderArea
			vk::Offset2D { 0, 0 },
			swapchainImageExtent 
		},
		1,											//clearValueCount
		&clearValue							//pClearValues
	};
	
	_commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

	_commandBuffer.bindVertexBuffers( 0, *vertexBuffer, { 0 } );
	_commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);

	vk::Viewport viewport{
		0.0f,											//x
		0.0f,											//y
		static_cast<float>(swapchainImageExtent.width), //width
		static_cast<float>(swapchainImageExtent.height),//height
		0.0f,											//minDepth
		1.0f											//maxDepth
	};
	_commandBuffer.setViewport(0, viewport);

	vk::Rect2D scissor{
		VkOffset2D {0, 0},		//offset
		swapchainImageExtent	//extent
	};
	_commandBuffer.setScissor(0, scissor);
	
	_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, *descriptorSet, nullptr);

	updateCells();

	_commandBuffer.drawIndexed(indices.size(), ubo.grid_size.x * ubo.grid_size.y, 0, 0, 0);

	_commandBuffer.endRenderPass();

	_commandBuffer.end();
}

int Renderer::rateDeviceSuitability(const vk::raii::PhysicalDevice _device)
{
	//if all required queue families aren't found, don't use the device
	if (!queryQueueFamilyIndices(_device).isComplete())
	{
		return -1;
	}
	if (!checkDeviceExtensionSupport(_device))
	{
		return -2;
	}

	SwapchainSupportDetails SwapchainDetails = querySwapchainSupport(_device);
	bool swapchainAdequate = 
		!SwapchainDetails.formats.empty() || !SwapchainDetails.presentModes.empty();
	if (!swapchainAdequate)
	{
		return -3;
	}

	int score = 1;

	vk::PhysicalDeviceProperties properties = _device.getProperties();
	vk::PhysicalDeviceFeatures features = _device.getFeatures();


	if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
	{
		score += 1000;
	}
	if (queryQueueFamilyIndices(_device).graphicsFamily == queryQueueFamilyIndices(_device).presentFamily)
	{
		// better performance if graphics and presentation is done on the same queue
		score += 50;
		// It may actually be faster with multiple queues running concurrently but that would require manual tranfer of ownership. Look into it later.
	}

	return score;
}

//Returns the indices of all required Queue Families supported by the device.
Renderer::QueueFamilyIndices Renderer::queryQueueFamilyIndices(vk::raii::PhysicalDevice _device)
{
	QueueFamilyIndices indices;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties = _device.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		if (queueFamilyProperties.at(i).queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		vk::Bool32 presentSupport = false;
		presentSupport = _device.getSurfaceSupportKHR(i, *surface);
		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}
	}

	return indices;
}

//Returns the details of the Swapchain provided by the device.
Renderer::SwapchainSupportDetails Renderer::querySwapchainSupport(vk::raii::PhysicalDevice _device)
{
	SwapchainSupportDetails details;

	details.capabilities = _device.getSurfaceCapabilitiesKHR(*surface);
	details.formats = _device.getSurfaceFormatsKHR(*surface);
	details.presentModes = _device.getSurfacePresentModesKHR(*surface);

	return details;
}

//Check if Device Extensions required by the application are supported by the device.
bool Renderer::checkDeviceExtensionSupport(vk::raii::PhysicalDevice _device)
{
	//Get all extensions supported by the device
	std::vector<vk::ExtensionProperties> availableExtensions = _device.enumerateDeviceExtensionProperties();

	//Make a set of all required extensions and remove them from the set if they're found. If all required extensions are found, the set is empty.
	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());
	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

//Returns the Surface Format to be used by the Swapchain.
vk::SurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> _surfaceFormats)
{
	for (const vk::SurfaceFormatKHR& surfaceFormat : _surfaceFormats)
	{
		//if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return surfaceFormat;
		}
	}

	return _surfaceFormats[0];
}

//Returns the way in which the Swapchain present images to the screen (Vsync, Triple Buffering etc.)
vk::PresentModeKHR Renderer::choostSwapPresentMode(std::vector<vk::PresentModeKHR> _presentModes)
{
	for (const vk::PresentModeKHR& presentMode : _presentModes)
	{
		if (presentMode == vk::PresentModeKHR::eMailbox)
		{
			//if Triple Buffering is supported, use it.
			return presentMode;
		}
	}

	//VSync (is required to be supported by Vulkan)
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR _capabilities)
{
	if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return _capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		vk::Extent2D out = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		out.width = std::clamp(out.width, _capabilities.maxImageExtent.width, _capabilities.minImageExtent.width);
		out.height = std::clamp(out.height, _capabilities.maxImageExtent.height, _capabilities.minImageExtent.height);

		return out;
	}
}

//Check if Instance Extensions required by the application are supported by the Vulkan Implementation and return them.
std::vector<const char*> Renderer::getRequiredInstanceExtensions()
{
	//Gathering extensions required by GLFW
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//Lambda to check if the required instance extension is supported by the Vulkan implementation.
	auto checkInstanceExtensionSupport = [](const char* pExtension)
	{
		std::vector<vk::ExtensionProperties> availableExtensions;
		availableExtensions = vk::enumerateInstanceExtensionProperties();

		for (const auto& avail : availableExtensions) {
			if (strcmp(pExtension, avail.extensionName) == 0) {
				return true;
			}
		}

		return false;
	};

	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		if (!checkInstanceExtensionSupport(glfwExtensions[i]))
		{
			throw std::runtime_error("[VK_Instance]: Extensions required by GLFW not available!");
		}
	}

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		if (!checkInstanceExtensionSupport(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		{
			throw std::runtime_error("[VK_Instance]: VK_EXT_debug_utils not available!");
		}
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		if (checkInstanceExtensionSupport(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
		{
			extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}
		else {
			std::cout << "[VK_Instance]: VK_KHR_get_physical_device_properties2 couldn't be loaded. Find stack trace to this function lol.";
		}
		// need to include this extension to prevent this warning(possibly wrong):
		// [Validation Layer]: vkGetPhysicalDeviceProperties2KHR: Emulation found unrecognized structure type in pProperties->pNext - this struct will be ignored
		// Genuinely think this is a bug with the vulkanSDK cuz it throws on the vkCreateDevice call which then calls vkGetPhysicalDeviceProperties
	}

	return extensions;
}

bool Renderer::checkValidationLayerSupport()
{
	std::vector<vk::LayerProperties> availableLayers;
	availableLayers = vk::enumerateInstanceLayerProperties();

	/*uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());*/

	for (const auto& layer : ValidationLayers)
	{
		bool found = false;
		for (const auto& avail : availableLayers) {
			if (strcmp(layer, avail.layerName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) return false;
	}

	return true;
}

void Renderer::setupDebugMessenger()
{
	if (!enableValidationLayers) 
		return;

	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	debugUtilsMessenger = vk::raii::DebugUtilsMessengerEXT(instance, createInfo);
}

std::vector<char> Renderer::readFile(const std::filesystem::path& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("[CPU]: Failed to open file at: ") + filename.string());
	}

	std::cout << "[CPU]: Reading: " + filename.string() << "\n";
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

vk::raii::ShaderModule Renderer::createShaderModule(std::vector<char>& code)
{
	vk::ShaderModuleCreateInfo createInfo {
		vk::ShaderModuleCreateFlags{},							//flags
		code.size(),											//codeSize
		reinterpret_cast<const uint32_t*>(code.data())			//pCode
	};

	vk::raii::ShaderModule shaderModule(device, createInfo);

	return shaderModule;
}
