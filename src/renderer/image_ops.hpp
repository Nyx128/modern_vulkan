#include "../core/vk_ctx.hpp"

/*
this header file will contain image resource/layout manipulation operations
also loading functions and any other utility
*/

void transition_image_layout(vk::CommandBuffer& command_buffer, vk::Image& image,
	vk::ImageLayout old_layout, vk::ImageLayout new_layout,
	vk::AccessFlags src_access_mask, vk::AccessFlags dst_access_mask,
	vk::PipelineStageFlags src_stage, vk::PipelineStageFlags dst_stage) {

	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = 1;

	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = access;

	barrier.srcAccessMask = src_access_mask;
	barrier.dstAccessMask = dst_access_mask;

	command_buffer.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
}
