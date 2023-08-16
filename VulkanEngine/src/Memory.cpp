#include "Memory.h"

uint32_t vkUtil::FindMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
	/*
	* // Provided by VK_VERSION_1_0
		typedef struct VkPhysicalDeviceMemoryProperties 
		{
			uint32_t        memoryTypeCount;
			VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
			uint32_t        memoryHeapCount;
			VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
		} VkPhysicalDeviceMemoryProperties;
	*/
	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		//bit i of supportedMemoryIndices is set if that memory type is supported by the device
		bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

		//propertyFlags holds all the memory properties supported by this memory type
		bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

		if (supported && sufficient) return i;
	}

	return 0;
}

void vkUtil::AllocatedBufferMemory(Buffer& buffer, const BufferInputChunk& input)
{
	/*
	// Provided by VK_VERSION_1_0
		typedef struct VkMemoryRequirements 
		{
			VkDeviceSize    size;
			VkDeviceSize    alignment;
			uint32_t        memoryTypeBits;
		} VkMemoryRequirements;
	*/
	vk::MemoryRequirements memoryRequirements = input.m_logicalDevice.getBufferMemoryRequirements(buffer.m_buffer);

	/*
	* // Provided by VK_VERSION_1_0
		typedef struct VkMemoryAllocateInfo 
		{
			VkStructureType    sType;
			const void*        pNext;
			VkDeviceSize       allocationSize;
			uint32_t           memoryTypeIndex;
		} VkMemoryAllocateInfo;
	*/
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryTypeIndex(
		input.m_physicalDevice,
		memoryRequirements.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	buffer.m_bufferMemory = input.m_logicalDevice.allocateMemory(allocInfo);
	input.m_logicalDevice.bindBufferMemory(buffer.m_buffer, buffer.m_bufferMemory, 0);
}

Buffer vkUtil::CreateBuffer(BufferInputChunk input)
{
	/*
	* // Provided by VK_VERSION_1_0
	typedef struct VkBufferCreateInfo 
	{
		VkStructureType        sType;
		const void*            pNext;
		VkBufferCreateFlags    flags;
		VkDeviceSize           size;
		VkBufferUsageFlags     usage;
		VkSharingMode          sharingMode;
		uint32_t               queueFamilyIndexCount;
		const uint32_t*        pQueueFamilyIndices;
	} VkBufferCreateInfo;
	*/

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.flags = vk::BufferCreateFlags();
	bufferInfo.size = input.m_size;
	bufferInfo.usage = input.m_usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	Buffer buffer{ input.m_logicalDevice.createBuffer(bufferInfo) };
	AllocatedBufferMemory(buffer, input);
	return buffer;
}