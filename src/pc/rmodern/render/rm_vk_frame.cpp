#include "rm_vk.h"

#include <stdexcept>

void rm_vk_frame::createFrame()
{
	/*** CREATE SYNC OBJECTS ***/

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.flags = 0;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mSemaphoreImageAvailable) != VK_SUCCESS ||
		vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mSemaphoreRenderFinished) != VK_SUCCESS ||
		vkCreateFence(mDevice, &fenceInfo, nullptr, &mFenceInFlight) != VK_SUCCESS)
		throw std::runtime_error("Could not create sync objects!!");

	/*** CREATE RESETTABLE COMMAND BUFFER ***/

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	if (vkAllocateCommandBuffers(mDevice, &allocInfo, &mCommandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Could not create resettable command buffer!!");
}

void rm_vk_frame::waitInFlight()
{
	vkWaitForFences(mDevice, 1, &mFenceInFlight, VK_TRUE, UINT64_MAX);
}

VkCommandBuffer rm_vk_frame::beginCommands()
{
	// RESET PRIMARY COMMAND BUFFER
	vkResetCommandBuffer(mCommandBuffer, 0);

	// RECORD COMMAND BUFFER
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

	vkBeginCommandBuffer(mCommandBuffer, &beginInfo);

	return mCommandBuffer;
}

void rm_vk_frame::submit(VkQueue queue)
{
	// END COMMAND BUFFER RECORDING
	vkEndCommandBuffer(mCommandBuffer);

	// SUBMIT TO QUEUE
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer;

	mRenderWaitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitSemaphores = &mSemaphoreImageAvailable;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = &mRenderWaitFlags;

	submitInfo.pSignalSemaphores = &mSemaphoreRenderFinished;
	submitInfo.signalSemaphoreCount = 1;

	vkResetFences(mDevice, 1, &mFenceInFlight);

	if (vkQueueSubmit(queue, 1, &submitInfo, mFenceInFlight) != VK_SUCCESS)
		throw std::runtime_error("Could not submit command buffer to graphics queue!!");
}

VkResult rm_vk_frame::getNextImage(VkSwapchainKHR swapchain, uint32_t* imageIndex)
{
	return vkAcquireNextImageKHR(mDevice, swapchain, UINT64_MAX, mSemaphoreImageAvailable, VK_NULL_HANDLE, imageIndex);
}

VkResult rm_vk_frame::present(VkQueue presentQueue, uint32_t imageIndex, VkSwapchainKHR swapchain)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pResults = nullptr;

	presentInfo.pWaitSemaphores = &mSemaphoreRenderFinished;
	presentInfo.waitSemaphoreCount = 1;

	VkSwapchainKHR swapchains[] = { swapchain };
	presentInfo.pSwapchains = swapchains;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &imageIndex;

	return vkQueuePresentKHR(presentQueue, &presentInfo);
}

void rm_vk_frame::cleanup()
{
	vkDestroyFence(mDevice, mFenceInFlight, nullptr);
	vkDestroySemaphore(mDevice, mSemaphoreRenderFinished, nullptr);
	vkDestroySemaphore(mDevice, mSemaphoreImageAvailable, nullptr);

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mCommandBuffer);
}

void rm_vk_frame::init(rm_rapi_vk* renderAPI)
{
	if (initialized && renderAPI->mDevice != mDevice)
		throw std::runtime_error("rm_vk_frame class is already initialized!!");

	mDevice = renderAPI->mDevice;
	mCommandPool = renderAPI->mResettableCommandPool;
	initialized = true;
}

void rm_vk_frame::deInit()
{
	mDevice = VK_NULL_HANDLE;
	mCommandPool = VK_NULL_HANDLE;
	initialized = false;
}