#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>
#include <optional>

#include <SDL_video.h>

class VkCtx {
public:
    VkCtx(const VkCtx&) = delete;
    VkCtx& operator=(const VkCtx&) = delete;
    VkCtx(VkCtx&&) = default;
    VkCtx& operator=(VkCtx&&) = default;

    VkCtx(SDL_Window* _window, std::string _app_name);
    ~VkCtx();

    inline vk::Instance& get_instance() { return instance; }
    inline vk::SurfaceKHR& get_surface() { return surface; }
    inline vk::Device& get_device() { return device; }
    inline vk::Queue& get_graphics_queue() { return graphics_queue; }
    inline vk::Queue& get_compute_queue() { return compute_queue; }
    inline vk::Queue& get_present_queue() { return present_queue; }

    inline uint32_t get_graphics_qfamily() { return graphics_qfamily.value(); }
    inline uint32_t get_present_qfamily() { return present_qfamily.value(); }
    inline uint32_t get_compute_qfamily() { return compute_qfamily.value(); }

    inline vk::SwapchainKHR& get_swapchain() { return swapchain; }
    inline vk::Format get_swapchain_format() { return swapchain_format.format; }
    inline vk::Extent2D get_swapchain_extent() { return swapchain_extent; }

    //create a single primary command buffer
    vk::CommandBuffer create_pcommand_buffer(vk::CommandPool& pool);

private:
    void init_instance();
    void init_surface();
    void select_device();
    void select_queue_families();
    void init_device();
    void init_swapchain();

private:
    std::string app_name;
    SDL_Window* window;

    //never ever am i loading functions c style
    vk::detail::DynamicLoader dl;
    vk::Instance instance;

#ifdef _DEBUG
    vk::DebugUtilsMessengerEXT debug_messenger;
#endif

    std::vector<const char*> layers = {
#ifdef _DEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
    };
    
    std::vector<const char*> extensions = {
#ifdef _DEBUG
       VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };
    
    std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

    vk::PhysicalDeviceFeatures device_features;

    vk::SurfaceKHR surface;

    vk::PhysicalDevice physical_device;

    std::optional<uint32_t> graphics_qfamily;
    std::optional<uint32_t> present_qfamily;
    std::optional<uint32_t> compute_qfamily;

    vk::Device device;

    vk::Queue graphics_queue;
    vk::Queue present_queue;
    vk::Queue compute_queue;

    vk::SwapchainKHR swapchain;
    vk::SurfaceFormatKHR swapchain_format;
    vk::PresentModeKHR swapchain_pm;
    vk::Extent2D swapchain_extent;
    uint32_t swapchain_ic = 1;

};
