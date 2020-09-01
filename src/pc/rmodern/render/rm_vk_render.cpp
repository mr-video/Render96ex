#include "rm_vk.h"

#include <stdexcept>

void rm_rapi_vk::beginFrame()
{
    // WAIT OUR TURN
    mFrames[currentFrame].waitInFlight();

    // GET THE IMAGE, WAIT FOR IT TO BE AVAILABLE
    mSwapchainResult = mFrames[currentFrame].getNextImage(mSwapchain, &mImageIndex);

    if (mSwapchainResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }
    else if (mSwapchainResult != VK_SUCCESS && mSwapchainResult != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Could not acquire next image!!");

    mSwapImages[mImageIndex].waitInFlight();
    mSwapImages[mImageIndex].setCurrentFrame(&mFrames[currentFrame]);

    /*** OBTAIN, RECORD, AND SUBMIT A COMMAND BUFFER ***/

    mRenderCommandBuffer = mFrames[currentFrame].beginCommands();
        vkCmdBindPipeline(mRenderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
        mSwapImages[mImageIndex].beginRenderPass(mRenderCommandBuffer);
}

VkCommandBuffer rm_rapi_vk::getRenderCommandBuffer()
{
    return mRenderCommandBuffer;
}

void rm_rapi_vk::completeFrame()
{
         mSwapImages[mImageIndex].endRenderPass(mRenderCommandBuffer);
    mFrames[currentFrame].submit(mGraphicsQueue);

    // PRESENTING THE FRAME TO THE WINDOW
    mFrames[currentFrame].present(mPresentQueue, mImageIndex, mSwapchain);

    if (mSwapchainResult == VK_ERROR_OUT_OF_DATE_KHR || mSwapchainResult == VK_SUBOPTIMAL_KHR || mWAPI->wasWindowResized(true))
        recreateSwapchain();
    else if (mSwapchainResult != VK_SUCCESS)
        throw std::runtime_error("Could not present swapchain!!");

    // Update current frame
    currentFrame++;
    if (currentFrame >= MAX_FRAMES_IN_FLIGHT)
        currentFrame = 0;
}