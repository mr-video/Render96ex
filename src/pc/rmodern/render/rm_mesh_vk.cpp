#include "rm_vk.h"

#include <stdint.h>
#include <stdexcept>

void rm_mesh_vk::preload(uint32_t numVertices, rm_vtx* vertices, uint32_t numIndices, uint32_t* indices)
{
    mNumVertices = numVertices;
    mNumIndices = numIndices;

    /*** CREATE AND FILL VERTEX BUFFER ***/

    VkDeviceSize bufferSize = sizeof(rm_vtx) * mNumVertices;

    // Create and fill staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mRAPI->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMemory);

    // Now create the actual veretex buffer
    mRAPI->createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mVertexBuffer, &mVertexBufferMemory);

    mRAPI->copyBuffer(bufferSize, stagingBuffer, mVertexBuffer);

    vkDestroyBuffer(mRAPI->mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mRAPI->mDevice, stagingBufferMemory, nullptr);

    /*** CREATE AND FILL INDEX BUFFER ***/

    bufferSize = sizeof(indices[0]) * mNumIndices;

    // Create and fill staging buffer
    mRAPI->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMemory);

    // Now create the actual veretex buffer
    mRAPI->createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mIndexBuffer, &mIndexBufferMemory);

    mRAPI->copyBuffer(bufferSize, stagingBuffer, mIndexBuffer);

    vkDestroyBuffer(mRAPI->mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mRAPI->mDevice, stagingBufferMemory, nullptr);
}

void rm_mesh_vk::render()
{
    // TODO: move rendering to a secondary command buffer(?)
    VkCommandBuffer commandBuffer = mRAPI->getRenderCommandBuffer();

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, mNumIndices, 1, 0, 0, 0);
}

void rm_mesh_vk::cleanup()
{
    vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
    vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
    vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);
    vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);

    mRAPI = nullptr;
    mDevice = VK_NULL_HANDLE;
	initialized = false;
}

void rm_mesh_vk::init(rm_rapi_vk* renderAPI)
{
    if (initialized && renderAPI != mRAPI)
		throw std::runtime_error("Already initialized!!");

	mRAPI = renderAPI;
    mDevice = renderAPI->mDevice;
	initialized = true;
}

void rm_mesh_vk::activate()
{

}

void rm_mesh_vk::deactivate()
{

}