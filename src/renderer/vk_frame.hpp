#pragma once

#include "../core/vk_ctx.hpp"
#include "vk_shader.hpp"

#include <memory>

class VkFrame {
public:
	VkFrame(VkCtx& _ctx, vk::CommandBuffer _command_buffer);
	~VkFrame();

	VkFrame(const VkFrame&) = delete;
	VkFrame& operator=(const VkFrame&) = delete;
	VkFrame(VkFrame&&) = default;
	VkFrame& operator=(VkFrame&&) = default;

	inline vk::CommandBuffer& get_command_buffer() { return command_buffer; }
	
	void record_command_buffer(uint32_t img_index, std::unique_ptr<VkShader>& shader);

	//to manually clean the handles, ive made is so because when the frame is moved, the destructor is called
	void clean(vk::CommandPool& pool);

	vk::Fence render_fence;
	vk::Semaphore image_semaphore;

private:
	void init_render_info();
	void init_color_attachment(uint32_t img_index);
	void set_render_params();
private:
	VkCtx& ctx;

	vk::CommandBuffer command_buffer;

	vk::RenderingAttachmentInfoKHR color_attachment;
	vk::RenderingInfoKHR render_info;

};
