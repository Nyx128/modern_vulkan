#pragma once
#include "vk_frame.hpp"
#include "vk_shader.hpp"

class VkRenderer {
public:
	VkRenderer(VkCtx& _ctx);
	~VkRenderer();

	VkRenderer(const VkRenderer&) = delete;
	VkRenderer& operator=(const VkRenderer&) = delete;
	VkRenderer(VkRenderer&&) = default;
	VkRenderer& operator=(VkRenderer&&) = default;

	void render();

private:
	void init_present_semaphores();
	void handle_invalid_swapchain();
private:
	VkCtx& ctx;
	std::vector<VkFrame> swapchain_frames;
	std::vector<vk::Semaphore> present_semaphores;

	//it will be a common pattern passing shaders, as unique ptr references
	std::unique_ptr<VkShader> tri_shader;

	vk::CommandPool render_pool;

	vk::Queue graphics_queue = ctx.get_graphics_queue();
	vk::Queue present_queue = ctx.get_present_queue();
	vk::Device device = ctx.get_device();
	vk::SwapchainKHR swapchain = ctx.get_swapchain();

	const int flight_frames = 1;

	uint32_t frame_index = 0;

	int render_timeout = UINT64_MAX;
};
