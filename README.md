# modern\_vulkan

A demo application showcasing **modern Vulkan** capabilities. This project serves as a learning tool and reference for utilizing Vulkan 1.3 features like dynamic rendering, descriptor indexing (bindless), and other up-to-date rendering techniques.

> ⚠️ This project is **Windows-only** for now.

---

## 🔧 Features

* Vulkan 1.3+ API usage
* SDL2 for window and input handling
* GLM for math operations
* Dear ImGui for GUI overlay (included as a submodule)
* Premake5 build system
* Clean, modular C++17 codebase

---

## 📦 Requirements

* **Windows OS**
* **[Vulkan SDK (Full Version)](https://vulkan.lunarg.com/sdk/home)**
  Ensure it includes:
  * Vulkan 1.3 headers and loader
  * SDL2
  * GLM
  * VMA
* **Git (with submodule support)**
* **Premake5**

---

## 📁 Setup Instructions

### 1. Clone the repo with submodules

```bash
git clone --recurse-submodules https://github.com/your_username/modern_vulkan.git
```

If you already cloned it without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

---

### 2. Install Vulkan SDK

* Download and install the **latest Vulkan SDK** from [LunarG](https://vulkan.lunarg.com/sdk/home). Make sure to do the full installation includes sdl, glm and VMA
* Make sure the environment variables are correctly set (`VULKAN_SDK`, etc.).

---

### 3. Generate Project Files

```bash
premake5 vs2022
```

Then open the generated `.sln` file in Visual Studio.

---

### 4. Build and Run

* Build the solution in Visual Studio (x64 Debug or Release).
* Run the executable from the build directory.

---

## 🚧 Roadmap

* [ ] Linux support
* [ ] Advanced RenderGraph system
* [ ] Scene loading (GLTF, etc.)
* [ ] Compute shaders
* [ ] Multithreaded renderer

---

## 📚 References

* [Khronos Vulkan Guide](https://www.khronos.org/vulkan/)
* [Sascha Willems Vulkan Samples](https://github.com/SaschaWillems/Vulkan)
* [SDL2](https://github.com/libsdl-org/SDL)
* [GLM](https://github.com/g-truc/glm)
* [Dear ImGui](https://github.com/ocornut/imgui)

---

## 📝 License

MIT License. See `LICENSE` file for details.
