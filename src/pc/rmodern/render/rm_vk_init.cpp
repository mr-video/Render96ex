#include "rm_vk.h"

#include <glad/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>

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

	//*/
	if (enableValidationLayers)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);	/*

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
	vkInstCInfo.flags = 0;

	// if debugging, enable one validation layer and add the debug utils extension
	if (enableValidationLayers)
	{
		vkInstCInfo.enabledLayerCount = validationLayerNames.size();
		vkInstCInfo.ppEnabledLayerNames = validationLayerNames.data();

		reqExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
		vkInstCInfo.enabledLayerCount = 0;

	vkInstCInfo.enabledExtensionCount = reqExtensions.size();
	vkInstCInfo.ppEnabledExtensionNames = reqExtensions.data();

	VkResult result = vkCreateInstance(&vkInstCInfo, nullptr, &mInstance);
	if (result == VK_ERROR_LAYER_NOT_PRESENT) {
		throw std::runtime_error("Validation layer not present!");
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}

	loadVulkan(mInstance, VK_NULL_HANDLE, mVkGetInstanceProcAddr);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		|| (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
	{
		std::cerr << "VK Validation Layer: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}

void rm_rapi_vk::createDebugger()
{
	// If validation layers are enabled, create the debug messenger
	if (enableValidationLayers)
	{
		if(!vkCreateDebugUtilsMessengerEXT)
		{
			std::cerr << "vkCreateDebugUtilsMessengerEXT not present!!" << std::endl;
		}

		VkDebugUtilsMessengerCreateInfoEXT vkDebugCreateInfo = {};
		vkDebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		vkDebugCreateInfo.pfnUserCallback = debugCallback;
		vkDebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vkDebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vkDebugCreateInfo.pUserData = nullptr;

		VkResult result = vkCreateDebugUtilsMessengerEXT(mInstance, &vkDebugCreateInfo, nullptr, &mDebugMessenger);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Coult not create debug messenger!!");
		}
	}
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