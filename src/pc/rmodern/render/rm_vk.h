#ifndef RM_VK_H
#define RM_VK_H

#include "rm_rapi.h"
#include "../window/rm_wapi.h"
#include <glad/vulkan.h>
#include <vector>

class rm_rapi_vk;

extern const int MAX_FRAMES_IN_FLIGHT;

class rm_mesh_vk : public rm_mesh
{
public:
    virtual void preload(uint32_t numVertices, rm_vtx* vertices, uint32_t numIndices, uint32_t* indices);
    virtual void activate();
    virtual void deactivate();
    virtual void cleanup();
    virtual void render();

	void init(rm_rapi_vk* rapi);

private:
	uint32_t mNumVertices;
	uint32_t mNumIndices;

	bool initialized = false;
	rm_rapi_vk* mRAPI = nullptr;
	VkDevice mDevice = VK_NULL_HANDLE;

	VkBuffer mVertexBuffer = VK_NULL_HANDLE;
	VkBuffer mIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;
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
	friend GLADapiproc vkLoadFunc(const char *name);

public:
    virtual void setWAPI(rm_wapi* wapi);
    virtual bool checkSupport();
    virtual bool init();
    virtual void cleanup();
    virtual rm_mesh* createMesh();
    virtual uint32_t getRequiredWindowFlags();
	virtual void beginFrame();
	virtual void completeFrame();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    					VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory);
	void copyBuffer(VkDeviceSize size, VkBuffer src, VkBuffer dst);
	VkCommandBuffer getRenderCommandBuffer();

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

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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
	size_t currentFrame = 0;

	// Variables used in the process of rendering a frame
	VkResult mSwapchainResult;
	uint32_t mImageIndex;
	VkCommandBuffer mRenderCommandBuffer;
};

#endif
