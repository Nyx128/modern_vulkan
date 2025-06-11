#include "vk_frame.hpp"
#include "image_ops.hpp"
#include <iostream>

#include "imgui.h"

VkFrame::VkFrame(VkCtx& _ctx, GuiCtx& _gui_ctx, vk::CommandBuffer _command_buffer)
	:ctx(_ctx), gui_ctx(_gui_ctx), command_buffer(_command_buffer){
	auto device = ctx.get_device();
	vk::FenceCreateInfo fence_info;
	fence_info.flags = vk::FenceCreateFlagBits::eSignaled;
	render_fence = device.createFence(fence_info);

	vk::SemaphoreCreateInfo semaphore_info;
	image_semaphore = device.createSemaphore(semaphore_info);
}

void VkFrame::record_command_buffer(uint32_t img_index, std::unique_ptr<VkShader>& shader){
	auto device = ctx.get_device();
	gui_ctx.new_frame();
	command_buffer.reset();
	init_color_attachment(img_index);
	init_render_info();

	vk::CommandBufferBeginInfo begin_info;
	command_buffer.begin(begin_info);

	auto img = ctx.get_swapchain_images()[img_index];

	transition_image_layout(command_buffer, img, vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput);

	set_render_params();
	command_buffer.beginRenderingKHR(render_info);
	ImGui::ShowDemoWindow();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	vk::ShaderStageFlagBits stages[2] = {
		vk::ShaderStageFlagBits::eVertex,
		vk::ShaderStageFlagBits::eFragment
	};

	vk::ShaderEXT shaders[2] = {
		shader->get_vertex_shader(),
		shader->get_fragment_shader()
	};

	command_buffer.bindShadersEXT(stages, shaders);
	command_buffer.setVertexInputEXT(0, nullptr, 0, nullptr);

	command_buffer.draw(3, 1, 0, 0);
	gui_ctx.end_frame();
	command_buffer.endRendering();

	color_attachment.setLoadOp(vk::AttachmentLoadOp::eLoad);
	render_info.setColorAttachments(color_attachment)
		.setPDepthAttachment(nullptr);

	command_buffer.beginRendering(render_info);
	gui_ctx.render(command_buffer);

	command_buffer.endRendering();

	transition_image_layout(command_buffer, img,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eNone,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe);



	command_buffer.end();
}

void VkFrame::clean(vk::CommandPool& pool) {
	auto device = ctx.get_device();
	device.destroyFence(render_fence);
	device.destroySemaphore(image_semaphore);
	device.freeCommandBuffers(pool, 1, &command_buffer);
}

void VkFrame::init_render_info(){

	render_info.setFlags(vk::RenderingFlagsKHR());
	render_info.setRenderArea(vk::Rect2D({ 0,0 }, ctx.get_swapchain_extent()));
	render_info.setLayerCount(1);

	render_info.setViewMask(0);
	render_info.setColorAttachmentCount(1);
	render_info.setColorAttachments(color_attachment);
}

void VkFrame::init_color_attachment(uint32_t img_index){
	color_attachment.setImageView(ctx.get_swapchain_views()[img_index]);
	color_attachment.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
	color_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	color_attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	color_attachment.setClearValue(vk::ClearValue({ 0.01f, 0.01f, 0.02f, 1.0f }));
}

void VkFrame::set_render_params(){
	auto frame_size = ctx.get_swapchain_extent();
	vk::Viewport viewport =
		vk::Viewport(0.0f, 0.0f, frame_size.width, frame_size.height, 0.0f, 1.0f);
	command_buffer.setViewportWithCount(viewport);

	vk::Rect2D scissor = vk::Rect2D({ 0,0 }, frame_size);
	command_buffer.setScissorWithCount(scissor);

	command_buffer.setRasterizerDiscardEnable(0);
	command_buffer.setPolygonModeEXT(vk::PolygonMode::eFill);
	command_buffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
	uint32_t sampleMask = 1;
	command_buffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1, sampleMask);
	command_buffer.setAlphaToCoverageEnableEXT(0);
	command_buffer.setCullMode(vk::CullModeFlagBits::eNone);
	command_buffer.setDepthTestEnable(0);
	command_buffer.setDepthWriteEnable(0);
	command_buffer.setDepthBiasEnable(0);
	command_buffer.setStencilTestEnable(0);
	command_buffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
	command_buffer.setPrimitiveRestartEnable(0);
	uint32_t colorBlendEnable = 0;
	command_buffer.setColorBlendEnableEXT(0, colorBlendEnable);
	vk::ColorBlendEquationEXT equation;
	equation.colorBlendOp = vk::BlendOp::eAdd;
	equation.dstColorBlendFactor = vk::BlendFactor::eZero;
	equation.srcColorBlendFactor = vk::BlendFactor::eOne;
	command_buffer.setColorBlendEquationEXT(0, equation);
	vk::ColorComponentFlags colorWriteMask = vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB
		| vk::ColorComponentFlagBits::eA;
	command_buffer.setColorWriteMaskEXT(0, colorWriteMask);
}

VkFrame::~VkFrame(){
	
}
