#pragma once
#include "../core/vk_ctx.hpp"
#include "../core/vk_window.hpp"
#include "backends/imgui_impl_sdl2.h"

class GuiCtx {
public:
	GuiCtx(const GuiCtx&) = delete;
	GuiCtx& operator=(const GuiCtx&) = delete;
	GuiCtx(GuiCtx&&) = default;
	GuiCtx& operator=(GuiCtx&&) = default;

	GuiCtx(VkWindow& _window, VkCtx& _ctx);
	~GuiCtx();

	void new_frame();
	void end_frame();
	void render(vk::CommandBuffer& command_buffer);

	void process_events(SDL_Event& event) { ImGui_ImplSDL2_ProcessEvent(&event); }

private:
	enum class State : std::int8_t { Ended, Begun };
	State state = State::Ended;
private:
	VkWindow& window;
	VkCtx& ctx;
};