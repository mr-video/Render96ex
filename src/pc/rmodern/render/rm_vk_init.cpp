#include "rm_vk.h"

#include <glad/vulkan.h>
#include <vector>
#include <stdexcept>

#include "../window/rm_wapi.h"
#include "vk_helper.h"

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

std::vector<const char*> validationLayerNames = {
	"VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

rm_mesh* rm_rapi_vk::createMesh()
{
    return nullptr;
}

bool rm_rapi_vk::init()
{
	// checkSupport() also loads the vulkan library. It would be better to do this explicitly.
	if(!this->checkSupport())
		return false;

    createVkInstance();
	createDebugger();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
	createSwapchain();

	createRenderPass();
	createGraphicsPipeline();
	createCommandPools();

	createMesh();
	createFrames();
	createSwapImages();

    return true;
}

void rm_rapi_vk::cleanup()
{
    /*
    vkDeviceWaitIdle(mDevice);

	cleanupSwapchain();

	mMesh->cleanup();
	mMesh2->cleanup();

	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);

	for (FrameVK frame : mFrames)
		frame.cleanup();

	vkDestroyCommandPool(mDevice, mTransientCommandPool, nullptr);
	vkDestroyCommandPool(mDevice, mResettableCommandPool, nullptr);
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyDevice(mDevice, nullptr);  //*/
	vkDestroyInstance(mInstance, nullptr);
}

void rm_rapi_vk::createVkInstance()
{
    // get required extensions for the window
	std::vector<const char*> reqExtensions = mWAPI->getVulkanRequiredExtensions();

	// provide information about the app
	VkApplicationInfo vkAppInfo = {};
	vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.pApplicationName = "Vulkan Test";
	vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.pEngineName = "Super Mario 64";
	vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.apiVersion = VK_API_VERSION_1_0;

	// cool, now let's create a Vulkan instance
	VkInstanceCreateInfo vkInstCInfo = {};
	vkInstCInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstCInfo.pApplicationInfo = &vkAppInfo;
	vkInstCInfo.enabledExtensionCount = reqExtensions.size();
	vkInstCInfo.ppEnabledExtensionNames = reqExtensions.data();
	vkInstCInfo.flags = 0;

	// if debugging, enable one validation layer
	if (enableValidationLayers)
	{
		vkInstCInfo.enabledLayerCount = validationLayerNames.size();
		vkInstCInfo.ppEnabledLayerNames = validationLayerNames.data();
	}
	else
		vkInstCInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&vkInstCInfo, nullptr, &mInstance);
	if (result == VK_ERROR_LAYER_NOT_PRESENT) {
		throw std::runtime_error("Validation layer not present!");
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}

	loadVulkan(mInstance, VK_NULL_HANDLE, mVkGetInstanceProcAddr);
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