#ifndef RM_VK_H
#define RM_VK_H

#include "rm_rapi.h"
#include <glad/vulkan.h>
#include <vector>

class rm_rapi_vk;

class rm_mesh_vk : public rm_mesh
{
public:
    virtual void preload(std::vector<rm_vtx> vertices, std::vector<uint32_t> indices);
    virtual void activate();
    virtual void deactivate();
    virtual void cleanup();
    virtual void render();
};

class rm_vk_frame
{
public:
	void createFrame();
	void waitInFlight();
	VkResult getNextImage(VkSwapchainKHR swapchain, uint32_t* imageIndex);
	VkCommandBuffer beginCommands();
	void submit(VkQueue queue);
	VkResult present(VkQueue presentQueue, uint32_t imageIndex, VkSwapchainKHR swapchain);
	void cleanup();

	void init(rm_rapi_vk* renderAPI);
	void deInit();

private:
	VkFence mFenceInFlight;
	VkSemaphore mSemaphoreImageAvailable;
	VkSemaphore mSemaphoreRenderFinished;

	VkPipelineStageFlags mRenderWaitFlags;

	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
	VkCommandPool mCommandPool = VK_NULL_HANDLE;

	VkDevice mDevice;
	bool initialized = false;
};

class rm_vk_swapimage
{
public:
	void createSwapImage(VkImage image, VkFormat format, VkRenderPass renderPass, VkExtent2D extent);
	void waitInFlight();
	void setCurrentFrame(rm_vk_frame* frame);
	void beginRenderPass(VkCommandBuffer commandBuffer);
	void endRenderPass(VkCommandBuffer commandBuffer);
	void cleanup();

	void init(rm_rapi_vk* renderAPI);
	void deInit();

private:
	VkFramebuffer mFramebuffer;
	VkImage mImage;
	VkImageView mImageView;

	VkRenderPass mRenderPass;
	VkExtent2D mExtent;

	rm_vk_frame* mCurrentFrame = nullptr;

	VkCommandPool mCommandPool;
	VkDevice mDevice;
	bool initialized = false;
};

class rm_rapi_vk : public rm_rapi
{
    friend class rm_vk_frame;
    friend class rm_vk_swapimage;
    friend class rm_mesh_vk;

public:
    virtual void setWAPI(rm_wapi* wapi);
    virtual bool checkSupport();
    virtual bool init();
    virtual void cleanup();
    virtual rm_mesh* createMesh();
    virtual uint32_t getRequiredWindowFlags();

    friend GLADapiproc vkLoadFunc(const char *name);

private:
    void createVkInstance();
	void createDebugger();
	void selectPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapchain();
	void createRenderPass();
	void createFrames();
	void createSwapImages();
	void createGraphicsPipeline();
	void createCommandPools();
    void recreateSwapchain();
	void cleanupSwapchain();

    int rateDeviceSuitability(VkPhysicalDevice device);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    PFN_vkGetInstanceProcAddr mVkGetInstanceProcAddr = nullptr;
    rm_wapi* mWAPI = nullptr;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface;

    uint32_t mPresentQueueFamily;
    uint32_t mGraphicsQueueFamily;
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> mSwapchainImages;
    VkExtent2D mSwapchainExtent;
    VkFormat mSwapchainFormat;

    VkRenderPass mRenderPass;

    VkPipelineLayout mPipelineLayout;
    VkPipeline mGraphicsPipeline;

    VkCommandPool mLongtermCommandPool = VK_NULL_HANDLE;
    VkCommandPool mTransientCommandPool = VK_NULL_HANDLE;
    VkCommandPool mResettableCommandPool = VK_NULL_HANDLE;

    std::vector<rm_vk_frame> mFrames;
    std::vector<rm_vk_swapimage> mSwapImages;
};

#endif
