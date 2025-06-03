#include <SDL.h>
#include <SDL_vulkan.h>
#include <vector>
#include <string>

/// <summary>
/// wrapper class for SDL_Window, sets up window and handles input
/// </summary>

class VkWindow {
public:
    VkWindow(const std::string& title, int width, int height);

    ~VkWindow();

    // Polls SDL events and updates key state array
    void pollEvents();

    // Check if a specific key is currently pressed
    bool isKeyPressed(SDL_Scancode scancode) const {
        return keyStates[scancode];
    }

    // Returns whether the window should close
    bool windowShouldClose() const {
        return shouldClose;
    }

    // Access the raw SDL_Window pointer
    SDL_Window* getSDLWindow() const {
        return window;
    }

private:
    SDL_Window* window = nullptr;
    std::vector<bool> keyStates;
    bool shouldClose = false;
};
