#include "rmodern.h"

#include "render/rm_vk.h"
#include "window/rm_wapi_sdl.h"

#include <iostream>

#include <PR/gbi.h>
#include "../../../actors/common0.h"

void rmodern_init()
{
    rm_rapi* rapi = new rm_rapi_vk;
    rm_wapi* wapi = new rm_wapi_sdl;

    wapi->init();

    rapi->setWAPI(wapi);
    if(rapi->checkSupport())
        std::cout << "Vulkan is supported!!" << std::endl;
    else
        std::cout << "Vulkan is not supported :(" << std::endl;

    wapi->createWindow(rapi->getRequiredWindowFlags());
    rapi->init();

    rm_mesh* mesh = rapi->createMesh();
    mesh->preloadFromDL(flyguy_seg8_dl_08011420);
    
    while(wapi->handleEvents())
    {
        rapi->beginFrame();
        mesh->render();
        rapi->completeFrame();
    }

    mesh->cleanup();
    delete mesh;
    rapi->cleanup();
    wapi->destroyWindow();
    wapi->cleanup();
}