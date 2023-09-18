#pragma once

#include "vulkan/vulkan_core.h"
#include <iostream>
#include <vulkan/vulkan_raii.hpp>

//Called whenever a GLFW error occurs.
inline void glfwErrorCallback(int code, const char* description)
{
	std::cerr << "[GLFW]: " << code << ": " << description;
	std::cerr << std::endl;
}

//Called whenever a Vulkan throws an error/warning.
inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		std::cerr << "[Validation Layer]: " << pCallbackData->pMessage << "\n";
	}
	
	return VK_FALSE;
}

//Helper to fill the VkDebugUtilsMessengerCreateInfoEXT struct.
inline void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.setMessageSeverity(
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	createInfo.setMessageType(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	createInfo.setPfnUserCallback(debugCallback);
}