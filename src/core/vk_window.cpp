#include "vk_window.hpp"
#include "backends/imgui_impl_sdl2.h"

#include <stdexcept>

#include "../gui/gui_ctx.hpp"

VkWindow::VkWindow(const std::string& title, int width, int height){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("failed to initialize SDL");
    }

    // Create SDL window with Vulkan flag
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
    }

    // Initialize key state array
    keyStates.resize(SDL_NUM_SCANCODES, false);
}

void VkWindow::clean() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void VkWindow::set_gui_context(void* _gui_ctx){
    gui_ctx = _gui_ctx;
}

VkWindow::~VkWindow(){
    
}

void VkWindow::pollEvents(){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        reinterpret_cast<GuiCtx*>(gui_ctx)->process_events(event);
        //input handling
        if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            keyStates[event.key.keysym.scancode] = true;
        }
        if (event.type == SDL_KEYUP) {
            keyStates[event.key.keysym.scancode] = false;
        }
        if (event.type == SDL_QUIT) {
            shouldClose = true;
        }
    }
}
