#include "rm_vk.h"

#include <glad/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <set>

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

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
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
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);	//*/
	
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);	
	//vkDestroyDevice(mDevice, nullptr);
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
	// Let's find some physical devices
	uint32_t numPhysicalDevices;
	vkEnumeratePhysicalDevices(mInstance, &numPhysicalDevices, nullptr);
	VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[numPhysicalDevices];
	vkEnumeratePhysicalDevices(mInstance, &numPhysicalDevices, physicalDevices);

	int selectedDeviceSuitability = -1;

	// And check all of them out; try to find a discrete GPU
	for (uint32_t i = 0; i < numPhysicalDevices; i++)
	{
		// see if this device is better than the previously selected device
		int currentDeviceSuitability = rateDeviceSuitability(physicalDevices[i]);

		if (currentDeviceSuitability > selectedDeviceSuitability)
		{
			mPhysicalDevice = physicalDevices[i];
			selectedDeviceSuitability = currentDeviceSuitability;
		}
	}

	VkPhysicalDeviceProperties propertiess;
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &propertiess);
	std::cout << "Chose " << propertiess.deviceName << std::endl;

	if (mPhysicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Could not find a device with Vulkan support!!");

	delete[] physicalDevices;
}

// Priorities for different device types
const int deviceTypePriorities[] = {
	0,	// VK_PHYSICAL_DEVICE_TYPE_OTHER 
	3,	// VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU 
	4,	// VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
	2,	// VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU 
	1	// VK_PHYSICAL_DEVICE_TYPE_CPU 
};

bool checkDeviceExtensions(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, SwapchainSupportDetails* details)
{
	// query surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities);

	// clear vectors in passed struct
	details->formats.clear();
	details->presentModes.clear();

	// get supported surface formats
	uint32_t numFormats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, nullptr);
	if (numFormats > 0)
	{
		details->formats.resize(numFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, details->formats.data());
	}

	// get supported present modes
	uint32_t numModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numModes, nullptr);
	if (numModes > 0)
	{
		details->presentModes.resize(numModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numModes, details->presentModes.data());
	}
}

int rm_rapi_vk::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	// ensure device type is within bounds
	if (properties.deviceType < 0 || properties.deviceType > 4)
		return -1;

	// ensure device has the necessary queues and presentation support
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
	VkQueueFamilyProperties* queueFamilyProperties = new VkQueueFamilyProperties[numQueueFamilies];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilyProperties);

	VkBool32 presentSupport = VK_FALSE;

	uint64_t availableQueueFlags = 0;
	for (uint32_t i = 0; i < numQueueFamilies; i++)
	{
		availableQueueFlags |= queueFamilyProperties[i].queueFlags;

		if (presentSupport == VK_FALSE)
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
	}
	delete[] queueFamilyProperties;

	if (availableQueueFlags & VK_QUEUE_GRAPHICS_BIT == 0 || presentSupport == VK_FALSE)
		return -1;

	// query swapchain support
	SwapchainSupportDetails swapchainDetails;
	querySwapchainSupport(device, mSurface, &swapchainDetails);
	if (swapchainDetails.formats.empty() || swapchainDetails.presentModes.empty())
		return -1;

	// pull suitability from array based on device type
	return deviceTypePriorities[properties.deviceType];
}

void rm_rapi_vk::createLogicalDevice()
{

}

void rm_rapi_vk::createSurface()
{
	mSurface = mWAPI->getVulkanSurface(mInstance);
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