#include "vk_renderer.hpp"


VkRenderer::VkRenderer(VkCtx& _ctx):ctx(_ctx) {
	auto device = ctx.get_device();
	auto swapchain = ctx.get_swapchain();

	tri_shader = std::make_unique<VkShader>("src/shaders/triangle.vert.spv", "src/shaders/triangle.frag.spv", ctx);

	std::vector<vk::Image> swapchain_images = device.getSwapchainImagesKHR(swapchain);
	for (int i = 0;i < swapchain_images.size();i++) {
		swapchain_frames.emplace_back(swapchain_images[i], ctx);
	}

	vk::CommandPoolCreateInfo pool_ci;
	pool_ci.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	pool_ci.setQueueFamilyIndex(ctx.get_graphics_qfamily());
	
	render_pool = device.createCommandPool(pool_ci);

	for (int i = 0; i < swapchain_frames.size();i++) {
		swapchain_frames[i].set_command_buffer(ctx.create_pcommand_buffer(render_pool), tri_shader);
	}

}

VkRenderer::~VkRenderer(){
	auto device = ctx.get_device();
	for (auto& frame : swapchain_frames) {
		frame.clean(render_pool);
	}
	device.destroyCommandPool(render_pool);
}

void VkRenderer::render(){
	auto graphics_queue = ctx.get_graphics_queue();
	auto present_queue = ctx.get_present_queue();
	auto device = ctx.get_device();
	auto swapchain = ctx.get_swapchain();

	uint32_t imageIndex{ 0 };

	vk::SubmitInfo submitInfo = {};

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &swapchain_frames[0].get_command_buffer();
	graphics_queue.submit(submitInfo);

	graphics_queue.waitIdle();

	vk::PresentInfoKHR presentInfo = {};

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;

	presentInfo.pImageIndices = &imageIndex;

	present_queue.presentKHR(presentInfo);
	present_queue.waitIdle();
}
