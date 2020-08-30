#include "rm_vk.h"

#include <glad/vulkan.h>

#include "../window/rm_wapi.h"
#include "vk_helper.h"

void rm_rapi_vk::setWAPI(rm_wapi* wapi)
{
    this->mWAPI = wapi;
}

// TODO: add checks for physical device presence
bool rm_rapi_vk::checkSupport()
{
    if(this->mWAPI == nullptr)
        return false;

    this->mVkGetInstanceProcAddr = this->mWAPI->getVulkanLoader();

    if(!this->mVkGetInstanceProcAddr)
        return false;

    loadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, this->mVkGetInstanceProcAddr);

    if(vkCreateInstance)
        return true;
    else
        return false;
}

uint32_t rm_rapi_vk::getRequiredWindowFlags()
{
    return RM_WFLAG_VULKAN;
}

bool rm_rapi_vk::init()
{
    return false;
}

rm_mesh* rm_rapi_vk::createMesh()
{
    return nullptr;
}

void rm_rapi_vk::createVkInstance()
{

}

void rm_rapi_vk::createDebugger()
{

}

void rm_rapi_vk::selectPhysicalDevice()
{

}

void rm_rapi_vk::createLogicalDevice()
{

}

void rm_rapi_vk::createSurface()
{

}

void rm_rapi_vk::createSwapchain()
{

}

void rm_rapi_vk::createRenderPass()
{

}

void rm_rapi_vk::createFrames()
{

}

void rm_rapi_vk::createSwapImages()
{

}

void rm_rapi_vk::createGraphicsPipeline()
{

}

void rm_rapi_vk::createCommandPools()
{

}

void rm_rapi_vk::recreateSwapchain()
{

}

void rm_rapi_vk::cleanupSwapchain()
{

}