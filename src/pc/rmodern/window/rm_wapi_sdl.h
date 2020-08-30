#ifndef RM_WAPI_SDL_H
#define RM_WAPI_SDL_H

#include "rm_wapi.h"
#include <SDL2/SDL.h>

class rm_wapi_sdl : public rm_wapi
{
public:
    virtual bool checkSupport();
    virtual void init();
    virtual void createWindow(uint32_t flags);
    virtual void destroyWindow();
    virtual void cleanup();
    virtual bool handleEvents();
    
    virtual PFN_vkGetInstanceProcAddr getVulkanLoader();
    virtual VkSurfaceKHR getVulkanSurface(VkInstance instance);
    virtual void getVulkanResolution(VkExtent2D* extent);
    virtual std::vector<const char*> getVulkanRequiredExtensions();

private:
    bool tryLoadVulkan();

    SDL_Window* mWindow = nullptr;
    bool vulkanLoaded = false;
};

#endif