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
	VkCtx& ctx;
	std::vector<VkFrame> swapchain_frames;

	//it will be a common pattern passing shaders, as unique ptr references
	std::unique_ptr<VkShader> tri_shader;

	vk::CommandPool render_pool;
};
