--all you need on windows is to have vulkan sdk installed with full config and you should be able to run this
--if you dont want that just take SDL2 and glm includes and put them in the sdk include :)


workspace "modernvk"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "modernvk"

project "base"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir ("build/%{cfg.buildcfg}")
    objdir ("build/obj/%{cfg.buildcfg}")

    files { "src/**.h", "src/**.cpp" }

    includedirs {
        "%{os.getenv('VULKAN_SDK')}/Include",           -- Vulkan headers
        "%{os.getenv('VULKAN_SDK')}/Include/SDL2",      -- SDL2 headers
        "%{os.getenv('VULKAN_SDK')}/Include/glm"        -- GLM headers
    }

    libdirs {
        "%{os.getenv('VULKAN_SDK')}/Lib"                -- Vulkan, SDL2 libs
    }

    links { "SDL2main", "SDL2", "vulkan-1" }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
