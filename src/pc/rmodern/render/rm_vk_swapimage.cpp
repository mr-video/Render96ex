#include "rm_vk.h"

#include <stdexcept>

void rm_vk_swapimage::createSwapImage(VkImage image, VkFormat format, VkRenderPass renderPass, VkExtent2D extent)
{
	mImage = image;
	mRenderPass = renderPass;
	mExtent = extent;

	/*** CREATE IMAGE VIEW ***/

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image;
	imageViewInfo.format = format;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;

	if (vkCreateImageView(mDevice, &imageViewInfo, nullptr, &mImageView) != VK_SUCCESS)
		throw std::runtime_error("Could not create image view!!");

	/*** CREATE FRAMEBUFFER ***/

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pAttachments = &mImageView;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mFramebuffer) != VK_SUCCESS)
		throw std::runtime_error("Could not create framebuffer!!");
}

void rm_vk_swapimage::waitInFlight()
{
	if (mCurrentFrame != nullptr)
		mCurrentFrame->waitInFlight();
}

void rm_vk_swapimage::setCurrentFrame(rm_vk_frame* frame)
{
	mCurrentFrame = frame;
}

void rm_vk_swapimage::beginRenderPass(VkCommandBuffer commandBuffer)
{
	// BEGIN THE RENDER PASS
	VkRenderPassBeginInfo renderInfo = {};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderInfo.framebuffer = mFramebuffer;
	renderInfo.renderPass = mRenderPass;
	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderInfo.pClearValues = &clearColor;
	renderInfo.clearValueCount = 1;
	renderInfo.renderArea.offset = { 0, 0 };
	renderInfo.renderArea.extent = mExtent;
	vkCmdBeginRenderPass(commandBuffer, &renderInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void rm_vk_swapimage::endRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
}

void rm_vk_swapimage::cleanup()
{
	vkDestroyFramebuffer(mDevice, mFramebuffer, nullptr);
	vkDestroyImageView(mDevice, mImageView, nullptr);
	
}

void rm_vk_swapimage::init(rm_rapi_vk* renderAPI)
{
	if (initialized && renderAPI->mDevice != mDevice)
		throw std::runtime_error("MeshVK class is already initialized!!");

	mCommandPool = renderAPI->mLongtermCommandPool;
	mDevice = renderAPI->mDevice;

	initialized = true;
}

void rm_vk_swapimage::deInit()
{
	mCommandPool = VK_NULL_HANDLE;
	mDevice = VK_NULL_HANDLE;
	initialized = false;
}