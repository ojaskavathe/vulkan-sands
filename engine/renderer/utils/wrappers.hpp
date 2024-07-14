#include <vulkan/vulkan_raii.hpp>

/**
 * vk::raii::SwapchainKHR::acquireNextImageKHR without exceptions
 */
inline std::pair<vk::Result, uint32_t>
SwapchainNextImageWrapper(const vk::raii::SwapchainKHR &swapchain,
                          uint64_t timeout, vk::Semaphore semaphore,
                          vk::Fence fence) {
  uint32_t image_index;
  vk::Result result =
      static_cast<vk::Result>(swapchain.getDispatcher()->vkAcquireNextImageKHR(
          static_cast<VkDevice>(swapchain.getDevice()),
          static_cast<VkSwapchainKHR>(*swapchain), timeout,
          static_cast<VkSemaphore>(semaphore), static_cast<VkFence>(fence),
          &image_index));
  return std::make_pair(result, image_index);
}

/**
 * vk::raii::Queue::presentKHR without exceptions
 */
inline vk::Result QueuePresentWrapper(vk::raii::Instance &instance,
                                      const vk::raii::Queue &queue,
                                      const vk::PresentInfoKHR &present_info) {

  // throws Poco::NotFoundException, apparently a driver issue
  vk::Result result =
      static_cast<vk::Result>(queue.getDispatcher()->vkQueuePresentKHR(
          static_cast<VkQueue>(*queue),
          reinterpret_cast<const VkPresentInfoKHR *>(&present_info)));

  return static_cast<vk::Result>(result);
}
