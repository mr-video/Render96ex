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

private:
    SDL_Window* mWindow = nullptr;
};

#endif