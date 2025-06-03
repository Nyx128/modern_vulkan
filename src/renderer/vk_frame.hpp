#pragma once

#include "../core/vk_ctx.hpp"
#include "vk_shader.hpp"

#include <memory>

class VkFrame {
public:
	VkFrame(vk::Image& _img, VkCtx& _ctx);
	~VkFrame();

	VkFrame(const VkFrame&) = delete;
	VkFrame& operator=(const VkFrame&) = delete;
	VkFrame(VkFrame&&) = default;
	VkFrame& operator=(VkFrame&&) = default;

	inline vk::CommandBuffer& get_command_buffer() { return command_buffer; }

	void set_command_buffer(vk::CommandBuffer _command_buffer, std::unique_ptr<VkShader>& shader);

	//to manually clean the handles, ive made is so because when the frame is moved, the destructor is called
	void clean(vk::CommandPool& pool);

private:
	void init_render_info();
	void init_color_attachment();
	void set_render_params();
private:
	vk::Image& img;
	VkCtx& ctx;
	vk::ImageView img_view;

	vk::CommandBuffer command_buffer;

	vk::RenderingAttachmentInfoKHR color_attachment;
	vk::RenderingInfoKHR render_info;
};
