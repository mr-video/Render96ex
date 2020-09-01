#include "rm_vk.h"

#include <glad/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <set>
#include <algorithm>

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
    vkDeviceWaitIdle(mDevice);	//*/

	cleanupSwapchain();	/*

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
	
	vkDestroyDevice(mDevice, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);	
	vkDestroyInstance(mInstance, nullptr);
}

void rm_rapi_vk::cleanupSwapchain()
{
	/*
	for (SwapImageVK swapImage : mSwapImages)
		swapImage.cleanup();

	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);	//*/
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);	//*/

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
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

	// Now let's load vulkan functions for our physical device
	loadVulkan(mInstance, mPhysicalDevice, mVkGetInstanceProcAddr);
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
	// First let's grab some queue families
	uint32_t graphicsQueueFamily;
	uint32_t presentQueueFamily;
	bool foundGraphicsQueueFamily = false;
	bool foundPresentQueueFamily = false;
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &numQueueFamilies, nullptr);
	VkQueueFamilyProperties* queueFamilyProperties = new VkQueueFamilyProperties[numQueueFamilies];
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &numQueueFamilies, queueFamilyProperties);

	for (uint32_t i = 0; i < numQueueFamilies; i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueFamily = i;
			foundGraphicsQueueFamily = true;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &presentSupport);
		if (presentSupport)
		{
			presentQueueFamily = i;
			foundPresentQueueFamily = true;
		}
	}
	delete[] queueFamilyProperties;

	if (!foundPresentQueueFamily || !foundGraphicsQueueFamily)
		throw std::runtime_error("Could not find needed queue families!!");

	mPresentQueueFamily = presentQueueFamily;
	mGraphicsQueueFamily = graphicsQueueFamily;

	// Now let's specify a graphics queue and a presentation queue
	VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
	float queuePriority = 1.0f;
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
	presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	presentQueueCreateInfo.queueFamilyIndex = presentQueueFamily;
	presentQueueCreateInfo.queueCount = 1;
	presentQueueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceQueueCreateInfo queueCreateInfos[] = {
		graphicsQueueCreateInfo,
		presentQueueCreateInfo
	};

	// Specify needed physical device features
	VkPhysicalDeviceFeatures deviceFeatures = {};

	// Create the logical device
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
	deviceCreateInfo.queueCreateInfoCount = (graphicsQueueFamily == presentQueueFamily) ? 1 : 2;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();

	if (enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = validationLayerNames.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
	}
	else
		deviceCreateInfo.enabledLayerCount = 0;

	if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS)
		throw std::runtime_error("Could not create logical device!!");

	// Now let's obtain the queues
	vkGetDeviceQueue(mDevice, graphicsQueueFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, presentQueueFamily, 0, &mPresentQueue);
}

void rm_rapi_vk::createSurface()
{
	mSurface = mWAPI->getVulkanSurface(mInstance);
}

VkSurfaceFormatKHR selectSurfaceFormat(VkSurfaceFormatKHR* formats, uint32_t numFormats)
{
	int bestRating = -1;
	VkSurfaceFormatKHR bestFormat;

	for (int i = 0; i < numFormats; i++)
	{
		int rating = 0;

		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
			rating += 2;

		if (formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			rating += 1;

		if (rating > bestRating)
			bestFormat = formats[i];
	}

	return bestFormat;
}

VkPresentModeKHR selectPresentMode(VkPresentModeKHR* modes, uint32_t numModes)
{
	// Try to use mailbox
	for (uint32_t i=0; i<numModes; i++)
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			return VK_PRESENT_MODE_MAILBOX_KHR;

	// FIFO is guarunteed and acceptable
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D selectSurfaceExtent(VkSurfaceCapabilitiesKHR& capabilities, rm_wapi* window)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		// Attempt to use the actual window resolution
		VkExtent2D actualExtent;
		window->getVulkanResolution(&actualExtent);

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void rm_rapi_vk::createSwapchain()
{
	// First let's grab the support details
	SwapchainSupportDetails supportDetails;
	querySwapchainSupport(mPhysicalDevice, mSurface, &supportDetails);
	VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(supportDetails.formats.data(), supportDetails.formats.size());
	VkPresentModeKHR presentMode = selectPresentMode(supportDetails.presentModes.data(), supportDetails.presentModes.size());
	VkExtent2D surfaceExtent = selectSurfaceExtent(supportDetails.capabilities, mWAPI);

	// How many images do we want?
	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
	if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
		imageCount = supportDetails.capabilities.maxImageCount;

	// Now let's make a swapchain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.presentMode = presentMode;
	createInfo.surface = mSurface;
	createInfo.imageExtent = surfaceExtent;
	createInfo.minImageCount = imageCount;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = supportDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Specify queue families
	uint32_t queueFamilyIndices[] = { mGraphicsQueueFamily, mPresentQueueFamily };
	if (mGraphicsQueueFamily == mPresentQueueFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.queueFamilyIndexCount = 0;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		createInfo.queueFamilyIndexCount = 2;
	}

	// Cool now let's make a swapchain
	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
		throw std::runtime_error("Could not create swapchain!!");

	// And grab its images
	uint32_t numImages;
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &numImages, nullptr);
	mSwapchainImages.resize(numImages);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &numImages, mSwapchainImages.data());

	// Finally, set private variables
	mSwapchainExtent = surfaceExtent;
	mSwapchainFormat = surfaceFormat.format;
}

void rm_rapi_vk::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = mSwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        throw std::runtime_error("Could not create render pass!!");
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
