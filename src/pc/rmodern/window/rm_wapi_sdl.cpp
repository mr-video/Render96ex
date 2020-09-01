#include "rm_wapi_sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

// TODO: actually check for support
bool rm_wapi_sdl::checkSupport()
{
    return true;
}

void rm_wapi_sdl::init()
{
    SDL_Init(SDL_INIT_VIDEO);
}

void rm_wapi_sdl::createWindow(uint32_t flags)
{
    if(mWindow != nullptr)
        return;

    // Convert RM window flags into SDL window flags
    uint32_t SDLFlags = 0;
    if(flags & RM_WFLAG_VULKAN)
    {
        tryLoadVulkan();
        SDLFlags |= SDL_WINDOW_VULKAN;
    }
    if(flags & RM_WFLAG_OPENGL)
        SDLFlags |= SDL_WINDOW_OPENGL;

    mWindow = SDL_CreateWindow("RModern Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDLFlags);
}

bool rm_wapi_sdl::handleEvents()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            return false;
            break;
        case SDL_WINDOWEVENT_RESIZED:
            wasResized = true;
            break;
        }
    }

    return true;
}

void rm_wapi_sdl::waitUntilActive()
{
    SDL_Event event;
    SDL_WaitEvent(&event);
}

bool rm_wapi_sdl::wasWindowResized(bool reset)
{
    bool ret = wasResized;

    if(reset)
        wasResized = false;

    return ret;
}
 
void rm_wapi_sdl::destroyWindow()
{
    if(mWindow == nullptr)
        return;

    SDL_DestroyWindow(mWindow);

    mWindow = nullptr;
}

void rm_wapi_sdl::cleanup()
{
    if(vulkanLoaded)
        SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

bool rm_wapi_sdl::tryLoadVulkan()
{
    if(vulkanLoaded)
        return true;

    return (SDL_Vulkan_LoadLibrary(nullptr) == 0);
}
    
PFN_vkGetInstanceProcAddr rm_wapi_sdl::getVulkanLoader()
{
    if(!tryLoadVulkan())
        return nullptr;

    return (PFN_vkGetInstanceProcAddr) SDL_Vulkan_GetVkGetInstanceProcAddr();
}

VkSurfaceKHR rm_wapi_sdl::getVulkanSurface(VkInstance instance)
{
    if(!tryLoadVulkan())
        return VK_NULL_HANDLE;

    VkSurfaceKHR surface;
    SDL_Vulkan_CreateSurface(mWindow, instance, &surface);
    return surface;
}

std::vector<const char*> rm_wapi_sdl::getVulkanRequiredExtensions()
{
    unsigned int numExtensions = 0;
    SDL_Vulkan_GetInstanceExtensions(mWindow, &numExtensions, nullptr);
    std::vector<const char*> extensions(numExtensions);
    SDL_Vulkan_GetInstanceExtensions(mWindow, &numExtensions, extensions.data());
    return extensions;
}

void rm_wapi_sdl::getVulkanResolution(VkExtent2D* extent)
{
    int w, h;
    SDL_Vulkan_GetDrawableSize(mWindow, &w, &h);
    extent->width = w;
    extent->height = h;
}