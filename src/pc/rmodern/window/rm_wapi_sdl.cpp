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
        SDLFlags |= SDL_WINDOW_VULKAN;
    if(flags & RM_WFLAG_OPENGL)
        SDLFlags |= SDL_WINDOW_OPENGL;

    mWindow = SDL_CreateWindow("RModern Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDLFlags);
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
    SDL_Quit();
}
    
PFN_vkGetInstanceProcAddr rm_wapi_sdl::getVulkanLoader()
{
    int result = SDL_Vulkan_LoadLibrary(NULL);

    if(result != 0)
        return nullptr;

    return (PFN_vkGetInstanceProcAddr) SDL_Vulkan_GetVkGetInstanceProcAddr();
}