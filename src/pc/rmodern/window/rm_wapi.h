#ifndef RM_WAPI_H
#define RM_WAPI_H

#include <glad/vulkan.h>
#include <vector>

#define RM_WFLAG_VULKAN 1
#define RM_WFLAG_OPENGL 2
#define RM_WFLAG_DIRECT3D 4

class rm_wapi
{
public:
    virtual bool checkSupport() = 0;
    virtual void init() = 0;
    virtual void createWindow(uint32_t flags) = 0;
    virtual void destroyWindow() = 0;
    virtual void cleanup() = 0;
    virtual bool handleEvents() = 0;
    
    virtual PFN_vkGetInstanceProcAddr getVulkanLoader()
    {
        return nullptr;
    }
    virtual VkSurfaceKHR getVulkanSurface(VkInstance instance)
    {
        return VK_NULL_HANDLE;
    }
    virtual std::vector<const char*> getVulkanRequiredExtensions()
    {
        std::vector<const char*> ret;
        return ret;
    }
    virtual void getVulkanResolution(VkExtent2D* extent)
    {
        return;
    }
};

#endif