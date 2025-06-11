#include "vk_ctx.hpp"
#include <stdexcept>
#include <iostream>

#include <SDL_vulkan.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#ifdef _DEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData){
        std::cerr << "validation layer: " << pCallbackData->pMessage << "\n\n" << std::endl;
        return VK_FALSE;
    }
#endif

VkCtx::VkCtx(SDL_Window* _window, std::string _app_name):app_name(_app_name), window(_window){
    init_instance();
    init_surface();
    select_device();
    select_queue_families();
    init_device();
    init_swapchain();
}

VkCtx::~VkCtx() {
    device.waitIdle();
    for (int i = 0; i < swapchain_views.size();i++) {
        device.destroyImageView(swapchain_views[i]);
    }
    device.destroySwapchainKHR(swapchain);
    device.destroy();
    instance.destroySurfaceKHR(surface);
#ifdef _DEBUG
    instance.destroyDebugUtilsMessengerEXT(debug_messenger);
#endif
    instance.destroy();
}

void VkCtx::recreate_swapchain(){
    device.waitIdle();
    for (int i = 0; i < swapchain_views.size();i++) {
        device.destroyImageView(swapchain_views[i]);
    }
    swapchain_images.clear();
    swapchain_views.clear();
    device.destroySwapchainKHR(swapchain);
    init_swapchain();
}

vk::CommandBuffer VkCtx::create_pcommand_buffer(vk::CommandPool& pool){
    vk::CommandBufferAllocateInfo alloc_info;
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = pool;
    alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);

    return device.allocateCommandBuffers(alloc_info)[0];
}


void VkCtx::init_instance(){

    vk::ApplicationInfo app_info{
        "modernvk", 1,
        nullptr, 0,
        VK_API_VERSION_1_3
    };

    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    uint32_t layer_count;
    auto available_layers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : layers) {
        bool layerFound = false;
        for (const auto& layerProperties : available_layers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) throw std::runtime_error(std::string("failed to find required layer: ") + std::string(layerName));


    }

    //add the sdl2 required extensions to the list

    uint32_t ext_cnt;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_cnt, nullptr)) {
        throw std::runtime_error(SDL_GetError());
    }

    std::vector<const char*> sdl_exts(ext_cnt);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_cnt, sdl_exts.data())) {
        throw std::runtime_error(SDL_GetError());
    }

    for (const char* ext : sdl_exts) {
        extensions.push_back(ext);
    }

    std::vector available_extensions = vk::enumerateInstanceExtensionProperties();
    for (const char* extName : extensions) {
        bool extFound = false;
        for (const auto& ext : available_extensions) {
            if (strcmp(ext.extensionName, extName) == 0) {
                extFound = true;
            }
        }
        if (!extFound) throw std::runtime_error(std::string("failed to find required extension: ") + std::string(extName));
    }


    vk::InstanceCreateInfo instance_info{
        {}, &app_info
    };

    instance_info.setPEnabledLayerNames(layers);
    instance_info.setPEnabledExtensionNames(extensions);

#ifdef _DEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debug_ci(
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback,
        nullptr // pUserData
    );

    instance_info.pNext = &debug_ci;
#endif

    instance = vk::createInstance(instance_info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#ifdef _DEBUG
    debug_messenger = instance.createDebugUtilsMessengerEXT(debug_ci);
#endif
    
}

void VkCtx::init_surface(){
    VkSurfaceKHR c_surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), &c_surface)) {
        throw std::runtime_error(SDL_GetError());
    }
    surface = vk::SurfaceKHR(c_surface);
}

void VkCtx::select_device(){
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    uint32_t best_index=0;
    uint64_t max_vram = 0;

    //prioritize discrete gpus
    for (int i = 0; i < devices.size();i++) {
        auto device = devices[i];
        vk::PhysicalDeviceProperties props = device.getProperties();

        if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            vk::PhysicalDeviceMemoryProperties mem_props = device.getMemoryProperties();
            for (uint32_t j = 0; j < mem_props.memoryHeapCount; j++) {
                auto heap = mem_props.memoryHeaps[j];
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                    if (heap.size > max_vram) { max_vram = heap.size; }
                    best_index = i;
                }
            }
        }
    }
    

    //in case discrete gpu isnt available, try finding the gpu with the most vram
    if (max_vram == 0) {
        for (int i = 0; i < devices.size();i++) {
            auto device = devices[i];
            vk::PhysicalDeviceProperties props = device.getProperties();

            vk::PhysicalDeviceMemoryProperties mem_props = device.getMemoryProperties();
            for (uint32_t j = 0; j < mem_props.memoryHeapCount; j++) {
                auto heap = mem_props.memoryHeaps[j];
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                    if (heap.size > max_vram) { max_vram = heap.size; }
                    best_index = i;
                }
            }
        }
    }

    physical_device = devices[best_index];
    auto props = physical_device.getProperties();
    std::cout << "selected GPU: " << props.deviceName << std::endl;
    std::cout << "available VRAM: " << max_vram/(1024 * 1024) << " MiB" <<  std::endl;

}

void VkCtx::select_queue_families(){
    std::vector<vk::QueueFamilyProperties> queue_props = physical_device.getQueueFamilyProperties();
    for (int i = 0; i < queue_props.size();i++) {
        //idk but i had to do this
        bool graphics = static_cast<bool>(queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics);
        bool compute = static_cast<bool>(queue_props[i].queueFlags & vk::QueueFlagBits::eCompute);

        bool present = physical_device.getSurfaceSupportKHR(i, surface);

        //pick general purpose compute queues, i think they are generally better.
        //idk tweak this if you wish to experiment.
        if (graphics && present && compute) {
            graphics_qfamily = i;
            present_qfamily = i;
            compute_qfamily = i;
            break;
        }

        if (graphics && !graphics_qfamily.has_value()) { graphics_qfamily = i; }
        if (present && !present_qfamily.has_value()) { present_qfamily = i; }
        if (compute && !compute_qfamily.has_value()) { compute_qfamily = i; }

    }

    if (!graphics_qfamily.has_value()){
        throw std::runtime_error("failed to find a queue family supporting graphics operations");
    }

    if (!compute_qfamily.has_value()) {
        throw std::runtime_error("failed to find a queue family supporting compute operations");
    }

    if (!present_qfamily.has_value()) {
        throw std::runtime_error("failed to find a queue family supporting present operations");
    }

    std::cout << "SELECTED GRAPHICS QUEUE FAMILY: " << graphics_qfamily.value() << std::endl;
    std::cout << "SELECTED COMPUTE QUEUE FAMILY: " << compute_qfamily.value() << std::endl;
    std::cout << "SELECTED PRESENT QUEUE FAMILY: " << present_qfamily.value() << std::endl;
}

void VkCtx::init_device(){
    //add necessary physical device features and device extensions after checking if they are there

    std::vector<vk::ExtensionProperties> available_extensions = physical_device.enumerateDeviceExtensionProperties();
    for (const auto& name : device_extensions) {
        bool ext_found = false;
        for (const auto& ext : available_extensions) {
            if (strcmp(ext.extensionName, name) == 0) {
                ext_found = true;
            }
        }
        if (!ext_found) {
            throw std::runtime_error("failed to find required device extension: " + std::string(name));
        }
    }

    vk::DeviceCreateInfo device_ci = {};
    device_ci.setPEnabledFeatures(&device_features);
    device_ci.setPEnabledExtensionNames(device_extensions);
    std::vector<vk::DeviceQueueCreateInfo> queue_infos;

    bool single_queue_family = (graphics_qfamily.value() == present_qfamily.value()) && (present_qfamily.value()  == compute_qfamily.value());

    if (single_queue_family) {
        float queue_priority = 1.0f;
        vk::DeviceQueueCreateInfo queue_ci = {};
        queue_ci.setQueueFamilyIndex(graphics_qfamily.value())
            .setQueueCount(1)
            .setPQueuePriorities(&queue_priority);

        queue_infos.push_back(queue_ci);
    }
    else {
        float queue_priority = 1.0f;
        vk::DeviceQueueCreateInfo gqueue_ci = {};
        gqueue_ci.setQueueFamilyIndex(graphics_qfamily.value())
            .setQueueCount(1)
            .setPQueuePriorities(&queue_priority);

        vk::DeviceQueueCreateInfo pqueue_ci = {};
        gqueue_ci.setQueueFamilyIndex(present_qfamily.value())
            .setQueueCount(1)
            .setPQueuePriorities(&queue_priority);

        vk::DeviceQueueCreateInfo cqueue_ci = {};
        gqueue_ci.setQueueFamilyIndex(compute_qfamily.value())
            .setQueueCount(1)
            .setPQueuePriorities(&queue_priority);

        queue_infos.push_back(gqueue_ci);
        queue_infos.push_back(pqueue_ci);
        queue_infos.push_back(cqueue_ci);
    }

    device_ci.setQueueCreateInfos(queue_infos);

    vk::PhysicalDeviceShaderObjectFeaturesEXT shaderFeatures = vk::PhysicalDeviceShaderObjectFeaturesEXT(1);
    vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering = vk::PhysicalDeviceDynamicRenderingFeaturesKHR(1);

    //enable any physical device features
    auto device_features = vk::PhysicalDeviceFeatures();
    device_ci.setPEnabledFeatures(&device_features);
    device_ci.pNext = &shaderFeatures;
    shaderFeatures.pNext = &dynamic_rendering;

    device = physical_device.createDevice(device_ci);

    //VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

    graphics_queue = device.getQueue(graphics_qfamily.value(), 0);
    present_queue = device.getQueue(present_qfamily.value(), 0);
    compute_queue = device.getQueue(compute_qfamily.value(), 0);
}

void VkCtx::init_swapchain(){
    vk::SurfaceCapabilitiesKHR capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
    std::vector<vk::SurfaceFormatKHR> formats = physical_device.getSurfaceFormatsKHR(surface);
    std::vector<vk::PresentModeKHR> present_modes = physical_device.getSurfacePresentModesKHR(surface);

    //choose format and colorspace
    swapchain_format = formats[0];
    for (const auto& fmt : formats) {
        //modern screens support srgb so its good for that
        if (fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && fmt.format == vk::Format::eB8G8R8A8Srgb) {
            swapchain_format = fmt;
        }
    }

    //choose present mode, ideally mailbox if available or fifo as default
    swapchain_pm = vk::PresentModeKHR::eFifo;
    for (const auto& pm : present_modes) {
        if (pm == vk::PresentModeKHR::eMailbox) {
            swapchain_pm = pm;
        }
    }

    //see if the current viewport size will be usable with this swapchain
    if (capabilities.currentExtent.width != UINT32_MAX) {
        swapchain_extent = capabilities.currentExtent;
    }
    else {
        vk::Extent2D extent;

        extent.width = std::min(
            capabilities.maxImageExtent.width,
            std::max(capabilities.minImageExtent.width, extent.width)
        );

        extent.height = std::min(
            capabilities.maxImageExtent.height,
            std::max(capabilities.minImageExtent.height, extent.height)
        );

        swapchain_extent = extent;
    }

    //usually modern gpus have minimum image count of 2, so normally this will be 3
    swapchain_ic = std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);

    vk::SwapchainCreateInfoKHR swapchain_ci = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(),
        surface, swapchain_ic, swapchain_format.format, swapchain_format.colorSpace,
        swapchain_extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    swapchain_ci.imageSharingMode = vk::SharingMode::eExclusive;
    swapchain_ci.clipped = true;
    swapchain_ci.preTransform = capabilities.currentTransform;
    swapchain_ci.presentMode = swapchain_pm;

    swapchain = device.createSwapchainKHR(swapchain_ci);

    swapchain_images = device.getSwapchainImagesKHR(swapchain);
    swapchain_views.resize(swapchain_images.size());
    for (int i = 0; i < swapchain_views.size();i++) {
        vk::ImageViewCreateInfo img_view_ci;
        img_view_ci.image = swapchain_images[i];
        img_view_ci.viewType = vk::ImageViewType::e2D;
        img_view_ci.format = swapchain_format.format;
        img_view_ci.components.r = vk::ComponentSwizzle::eIdentity;
        img_view_ci.components.g = vk::ComponentSwizzle::eIdentity;
        img_view_ci.components.b = vk::ComponentSwizzle::eIdentity;
        img_view_ci.components.a = vk::ComponentSwizzle::eIdentity;
        img_view_ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        img_view_ci.subresourceRange.baseMipLevel = 0;
        img_view_ci.subresourceRange.levelCount = 1;
        img_view_ci.subresourceRange.baseArrayLayer = 0;
        img_view_ci.subresourceRange.layerCount = 1;

        swapchain_views[i] = device.createImageView(img_view_ci);
    }
}
