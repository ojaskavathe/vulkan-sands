#pragma once

// #ifndef VULKAN_HPP_NO_EXCEPTIONS
// 	#define VULKAN_HPP_NO_EXCEPTIONS
// #endif

// #ifndef VULKAN_HPP_ASSERT_ON_RESULT
// 	#define VULKAN_HPP_ASSERT_ON_RESULT
// #endif

#include <array>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <string>
#include <filesystem>

#include <glm/glm.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#ifdef _DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif // If in release mode, no validation layers are to be used.

class Engine
{
private:

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static vk::VertexInputBindingDescription getBindingDescription() {
			vk::VertexInputBindingDescription bindingDescription{
				0,
				sizeof(Vertex),
				vk::VertexInputRate::eVertex
			};

			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescription() {
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions;

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);
			attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;

			return attributeDescriptions;
		}
	};

	struct UniformBufferObject {
		glm::vec2 grid_size;
	};

	const std::vector<Vertex> vertices = {
		{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
		{{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
		{{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	const UniformBufferObject ubo = { glm::vec2(64) };

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

	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
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

	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;

	vk::raii::Buffer indexBuffer = nullptr;
	vk::raii::DeviceMemory indexBufferMemory = nullptr;

	vk::raii::Buffer uniformBuffer = nullptr;
	vk::raii::DeviceMemory uniformBuffersMemory = nullptr;

	vk::raii::DescriptorPool descriptorPool = nullptr;
	vk::raii::DescriptorSet descriptorSet = nullptr;

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
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createCommandBuffers();
	void createDescriptorPool();
	void createDescriptorSet();

	void createSyncObjects();

	void drawFrame();
	
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize _size, 
					vk::BufferUsageFlags _usage,
					vk::MemoryPropertyFlags _propertyFlags);
	
	void copyBuffer(vk::raii::Buffer& _src, vk::raii::Buffer& _dst, vk::DeviceSize _size);

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