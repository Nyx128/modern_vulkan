#include <SDL.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <thread>
#include <atomic>

//wrappers
#include "core/vk_window.hpp"
#include "core/vk_ctx.hpp"
#include "renderer/vk_renderer.hpp"

void run() {
    VkWindow window("modernvk", 1280, 720);

    VkCtx context(window.getSDLWindow(), "modernvk");
    VkRenderer renderer(context);

    while (!window.windowShouldClose()) {
        
        renderer.render();
        window.pollEvents();
    }

    context.get_device().waitIdle();
}

int main(int argc, char* argv[]) {
    
    try { 
        run();
    }
    catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;        
    }
}
