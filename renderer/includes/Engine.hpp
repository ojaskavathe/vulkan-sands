#pragma once

// #ifndef VULKAN_HPP_NO_EXCEPTIONS
// 	#define VULKAN_HPP_NO_EXCEPTIONS
// #endif

// #ifndef VULKAN_HPP_ASSERT_ON_RESULT
// 	#define VULKAN_HPP_ASSERT_ON_RESULT
// #endif

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <string>
#include <filesystem>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#ifdef _DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif // If in release mode, no validation layers are to be used.

class Engine
{
private:
	GLFWwindow* window;
	
	vk::raii::Context context;
	
	vk::raii::Instance instance = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device device = nullptr;

	vk::raii::SurfaceKHR surface = nullptr;

	vk::raii::SwapchainKHR swapchain = nullptr;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::raii::ImageView> swapchainImageViews;
	vk::Format swapchainImageFormat;
	vk::Extent2D swapchainImageExtent;

	std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

	vk::raii::Queue graphicsQueue = nullptr;
	vk::raii::Queue presentQueue = nullptr;

	vk::raii::RenderPass renderPass = nullptr;
	vk::raii::PipelineLayout pipelineLayout = nullptr;

	vk::raii::Pipeline graphicsPipeline = nullptr;

	vk::raii::CommandPool commandPool = nullptr;
	vk::raii::CommandBuffers commandBuffers = nullptr;

	uint32_t currentFrame = 0;
	std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
	std::vector<vk::raii::Fence> inFlightFences;

	vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger = nullptr;

	bool framebufferResized = false;

	const std::vector<const char*> ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
		//The Physical Device may not support all Queue Families.
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

public:
	Engine();
	void run();

private:
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanUp();

	void recreateSwapchain();

	void createInstance();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapchain();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();

	void drawFrame();

	void recordCommandBuffer(vk::raii::CommandBuffer& _commandBuffer, uint32_t _imageIndex);

	std::vector<const char*> getRequiredInstanceExtensions();

	int rateDeviceSuitability(vk::raii::PhysicalDevice _device);
	bool checkDeviceExtensionSupport(vk::raii::PhysicalDevice _device);
	QueueFamilyIndices queryQueueFamilyIndices(vk::raii::PhysicalDevice _device);
	SwapchainSupportDetails querySwapchainSupport(vk::raii::PhysicalDevice _device);

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> _surfaceFormats);
	vk::PresentModeKHR choostSwapPresentMode(std::vector<vk::PresentModeKHR> _presentModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR _capabilities);

	bool checkValidationLayerSupport();
	void setupDebugMessenger();

	static std::vector<char> readFile(const std::filesystem::path& filename);
	vk::raii::ShaderModule createShaderModule(std::vector<char>& code);
};