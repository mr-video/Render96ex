#include "rm_vk.h"

#include <stdexcept>

uint32_t rm_rapi_vk::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);

    for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0;
}

void rm_rapi_vk::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.pNext = nullptr;

    if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
        throw std::runtime_error("Could not create buffer!!");

    // Get the memory requirements for the buffer so we can allocate memory
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(mDevice, *buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, memory) != VK_SUCCESS)
        throw std::runtime_error("Could not allocate vertex buffer memory!!");

    vkBindBufferMemory(mDevice, *buffer, *memory, 0);
}

void rm_rapi_vk::copyBuffer(VkDeviceSize size, VkBuffer src, VkBuffer dst)
{
    VkCommandBuffer cmdBuffer;

    // First allocate a transient command buffer
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mTransientCommandPool;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(mDevice, &allocInfo, &cmdBuffer) != VK_SUCCESS)
        throw std::runtime_error("Could not allocate command buffer to copy buffer data!!");

    // Now record commands
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = VK_NULL_HANDLE;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Could not begin recording command buffer to copy buffer data!!");

    VkBufferCopy copyInfo = {};
    copyInfo.srcOffset = 0;
    copyInfo.dstOffset = 0;
    copyInfo.size = size;
    vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyInfo);

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
        throw std::runtime_error("Could not end recording command buffer to copy buffer data!!");

    // Now submit commands
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("Could not submit command to copy buffer data!!");
    vkQueueWaitIdle(mGraphicsQueue);

    // Cleanup
    vkFreeCommandBuffers(mDevice, mTransientCommandPool, 1, &cmdBuffer);
}