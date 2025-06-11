#include "vk_renderer.hpp"
#include <chrono>

VkRenderer::VkRenderer(VkCtx& _ctx, GuiCtx& _gui_ctx):ctx(_ctx), gui_ctx(_gui_ctx) {
	tri_shader = std::make_unique<VkShader>("src/shaders/triangle.vert.spv", "src/shaders/triangle.frag.spv", ctx);

	std::vector<vk::Image> swapchain_images = device.getSwapchainImagesKHR(swapchain);

	vk::CommandPoolCreateInfo pool_ci;
	pool_ci.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	pool_ci.setQueueFamilyIndex(ctx.get_graphics_qfamily());
	
	render_pool = device.createCommandPool(pool_ci);

	for (int i = 0;i < swapchain_images.size();i++) {
		swapchain_frames.emplace_back(ctx, gui_ctx, ctx.create_pcommand_buffer(render_pool));
	}

	init_present_semaphores();

	std::chrono::seconds timeout_seconds(4);

	// Convert 4 seconds to nanoseconds
	std::chrono::nanoseconds timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout_seconds);
	render_timeout = timeout_ns.count();
}

VkRenderer::~VkRenderer(){
	device.waitIdle();
	for (int i = 0; i < present_semaphores.size();i++) {
		device.destroySemaphore(present_semaphores[i]);
	}

	for (auto& frame : swapchain_frames) {
		frame.clean(render_pool);
	}
	device.destroyCommandPool(render_pool);
}

void VkRenderer::render(){
	VkFrame& frame = swapchain_frames[frame_index];

	//wait atmost 4 seconds for a frame to render
	auto wait = device.waitForFences(frame.render_fence, false, render_timeout);
	if (wait != vk::Result::eSuccess) {
		throw std::runtime_error("render fence failed waiting");
	}
	device.resetFences(frame.render_fence);

	//since vulkan-hpp treats out of date khr as an exception, we have to rely on the c api for handling it
	uint32_t img_index;
	auto img_acq = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frame.image_semaphore, VK_NULL_HANDLE, &img_index);
	
	if (img_acq == VK_ERROR_OUT_OF_DATE_KHR || img_acq == VK_SUBOPTIMAL_KHR) {
		handle_invalid_swapchain();
		return;
	}
	
	if (img_acq != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire next image from swapchain");
	}

	frame.record_command_buffer(img_index, tri_shader);

	vk::SubmitInfo submitInfo = {};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &frame.image_semaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &present_semaphores[img_index];
	submitInfo.pCommandBuffers = &frame.get_command_buffer();
	vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	submitInfo.pWaitDstStageMask = &wait_stage;
	graphics_queue.submit(submitInfo, frame.render_fence);

	vk::PresentInfoKHR present_info = {};

	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &present_semaphores[img_index];
	present_info.pImageIndices = &img_index;

	VkPresentInfoKHR p_info = static_cast<VkPresentInfoKHR>(present_info);
	auto result = vkQueuePresentKHR(present_queue, &p_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		handle_invalid_swapchain();
		return;
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present to surface");
	}
	
	frame_index = (frame_index + 1) % flight_frames;
}

void VkRenderer::init_present_semaphores(){
	present_semaphores.resize(swapchain_frames.size());
	for (int i = 0; i < present_semaphores.size();i++) {
		vk::SemaphoreCreateInfo semaphore_info;
		present_semaphores[i] = device.createSemaphore(semaphore_info);
	}
}

void VkRenderer::handle_invalid_swapchain(){
	device.waitIdle();
	for (int i = 0; i < present_semaphores.size();i++) {
		device.destroySemaphore(present_semaphores[i]);
	}
	for (auto& frame : swapchain_frames) {
		frame.clean(render_pool);
	}
	swapchain_frames.clear();
	present_semaphores.clear();

	ctx.recreate_swapchain();

	swapchain = ctx.get_swapchain();

	std::vector<vk::Image> swapchain_images = device.getSwapchainImagesKHR(swapchain);
	for (int i = 0;i < swapchain_images.size();i++) {
		swapchain_frames.emplace_back(ctx, gui_ctx, ctx.create_pcommand_buffer(render_pool));
	}

	init_present_semaphores();
	auto extent = ctx.get_swapchain_extent();
	printf("swapchain recreated, size: %d x %d\n", extent.width, extent.height);
}
