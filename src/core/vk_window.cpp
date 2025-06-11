#include "vk_window.hpp"

#include <stdexcept>

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

VkWindow::~VkWindow(){
    
}

void VkWindow::pollEvents(){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
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
