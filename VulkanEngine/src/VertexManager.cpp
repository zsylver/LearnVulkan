#include "VertexManager.h"

VertexManager::VertexManager() : m_offset{ 0 }
{
}

void VertexManager::Consume(MeshTypes type, std::vector<float> vertexData) 
{

	for (float attribute : vertexData) 
	{
		m_lump.push_back(attribute);
	}
	int vertexCount = static_cast<int>(vertexData.size() / 5);

	m_offsets.insert(std::make_pair(type, m_offset));
	m_sizes.insert(std::make_pair(type, vertexCount));

	m_offset += vertexCount;
}

void VertexManager::Finalize(const FinalizationChunk& finalizationChunk)
{
	m_logicalDevice = finalizationChunk.m_logicalDevice;

	BufferInputChunk inputChunk;
	inputChunk.m_logicalDevice = finalizationChunk.m_logicalDevice;
	inputChunk.m_physicalDevice = finalizationChunk.m_physicalDevice;
	inputChunk.m_size = sizeof(float) * m_lump.size();
	inputChunk.m_usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	Buffer stagingBuffer = vkUtil::CreateBuffer(inputChunk);

	void* memoryLocation = m_logicalDevice.mapMemory(stagingBuffer.m_bufferMemory, 0, inputChunk.m_size);
	memcpy(memoryLocation, m_lump.data(), inputChunk.m_size);
	m_logicalDevice.unmapMemory(stagingBuffer.m_bufferMemory);

	inputChunk.m_usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	m_vertexBuffer = vkUtil::CreateBuffer(inputChunk);

	vkUtil::CopyBuffer(stagingBuffer, m_vertexBuffer, inputChunk.m_size, finalizationChunk.m_queue, finalizationChunk.m_commandBuffer);

	m_logicalDevice.destroyBuffer(stagingBuffer.m_buffer);
	m_logicalDevice.freeMemory(stagingBuffer.m_bufferMemory);
}

VertexManager::~VertexManager() 
{
	m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
	m_logicalDevice.freeMemory(m_vertexBuffer.m_bufferMemory);
}