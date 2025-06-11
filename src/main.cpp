#include <SDL.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <thread>
#include <atomic>

//wrappers
#include "core/vk_window.hpp"
#include "core/vk_ctx.hpp"
#include "renderer/vk_renderer.hpp"

void run(VkWindow& window, std::atomic<bool>* done) {
    try {

        VkCtx context(window.getSDLWindow(), "modernvk");
        VkRenderer renderer(context);

        while (!*done) {
           renderer.render();

        }
    }
    catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return;
    }

}

int main(int argc, char* argv[]) {
    
    VkWindow window("modernvk", 1280, 720);
    try {
        std::atomic<bool> done = false;
        std::thread render_thread(run, window, &done);

        while (!window.windowShouldClose()) {
            window.pollEvents();
        }

        done = true;
        render_thread.join();
        window.clean();
        //for some reason, it gets stuck when attempting
        //clean in the destructor so i call it manually here
    }
    catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;        
    }
}
