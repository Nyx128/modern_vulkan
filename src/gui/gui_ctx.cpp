#include "gui_ctx.hpp"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"

#include "glm/glm.hpp"
#include "glm/gtc/color_space.hpp"

GuiCtx::GuiCtx(VkWindow& _window, VkCtx& _ctx):window(_window), ctx(_ctx){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	static auto const load_vk_func = +[](char const* name, void* user_data) {
		return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
			*static_cast<vk::Instance*>(user_data), name);
	};

	auto instance = ctx.get_instance();
	ImGui_ImplVulkan_LoadFunctions(ctx.get_api_version(), load_vk_func,
		&instance);

	if (!ImGui_ImplSDL2_InitForVulkan(window.getSDLWindow())) {
		throw std::runtime_error("failed to initialize sdl2 backend for imgui");
	}

	auto init_info = ImGui_ImplVulkan_InitInfo{};
	init_info.ApiVersion = ctx.get_api_version();
	init_info.Instance = ctx.get_instance();
	init_info.PhysicalDevice = ctx.get_physical();
	init_info.Device = ctx.get_device();
	init_info.QueueFamily = ctx.get_graphics_qfamily();
	init_info.Queue = ctx.get_graphics_queue();
	init_info.MinImageCount = 2;
	init_info.ImageCount = static_cast<std::uint32_t>(ctx.get_swapchain_images().size());
	init_info.MSAASamples =
		static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
	init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE+1;
	auto pipeline_rendering_ci = vk::PipelineRenderingCreateInfo{};

	pipeline_rendering_ci.setColorAttachmentCount(1);
	auto format = ctx.get_swapchain_format();
	pipeline_rendering_ci.setPColorAttachmentFormats(&format);

	init_info.PipelineRenderingCreateInfo = pipeline_rendering_ci;
	init_info.UseDynamicRendering = true;
	if (!ImGui_ImplVulkan_Init(&init_info)) {
		throw std::runtime_error{ "Failed to initialize Dear ImGui" };
	}
	
	ImGui::StyleColorsDark();
	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
	for (auto& colour : ImGui::GetStyle().Colors) {
		auto const linear = glm::convertSRGBToLinear(
			glm::vec4{ colour.x, colour.y, colour.z, colour.w });
		colour = ImVec4{ linear.x, linear.y, linear.z, linear.w };
	}
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.99f;
}

GuiCtx::~GuiCtx(){
	auto device = ctx.get_device();
	device.waitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void GuiCtx::new_frame(){
	if (state == State::Begun) { end_frame(); }
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplSDL2_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
	state = State::Begun;
}

void GuiCtx::end_frame(){
	if(state == State::Ended) { return; }
	ImGui::Render();
	state = State::Ended;
}

void GuiCtx::render(vk::CommandBuffer& command_buffer){
	auto* data = ImGui::GetDrawData();
	if (data == nullptr) { return; }
	ImGui_ImplVulkan_RenderDrawData(data, command_buffer);
}
