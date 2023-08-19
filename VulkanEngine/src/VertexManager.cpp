#include "VertexManager.h"

VertexManager::VertexManager() : m_indexOffset{ 0 }
{
}

void VertexManager::Consume(MeshTypes type, std::vector<float>& vertexData, std::vector<uint32_t>& indexData) 
{
	int vertexCount = static_cast<int>(vertexData.size() / 7);
	int indexCount = static_cast<int>(indexData.size());
	int lastIndex = static_cast<int>(m_indexLump.size());

	m_firstIndices.insert(std::make_pair(type, lastIndex));
	m_indexCounts.insert(std::make_pair(type, indexCount));

	for (float attribute : vertexData)
	{
		m_vertexLump.push_back(attribute);
	}

	for (uint32_t index : indexData)
	{
		m_indexLump.push_back(index + m_indexOffset);
	}

	m_indexOffset += vertexCount;
}

void VertexManager::Finalize(const FinalizationChunk& finalizationChunk)
{
	m_logicalDevice = finalizationChunk.m_logicalDevice;

	// Make a staging buffer for vertices
	BufferInputChunk inputChunk;
	inputChunk.m_logicalDevice = finalizationChunk.m_logicalDevice;
	inputChunk.m_physicalDevice = finalizationChunk.m_physicalDevice;
	inputChunk.m_size = sizeof(float) * m_vertexLump.size();
	inputChunk.m_usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	Buffer stagingBuffer = vkUtil::CreateBuffer(inputChunk);

	// Fill it with vertex data
	void* memoryLocation = m_logicalDevice.mapMemory(stagingBuffer.m_bufferMemory, 0, inputChunk.m_size);
	memcpy(memoryLocation, m_vertexLump.data(), inputChunk.m_size);
	m_logicalDevice.unmapMemory(stagingBuffer.m_bufferMemory);

	// Make the vertex buffer
	inputChunk.m_usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	m_vertexBuffer = vkUtil::CreateBuffer(inputChunk);

	// Fill it
	vkUtil::CopyBuffer(stagingBuffer, m_vertexBuffer, inputChunk.m_size, finalizationChunk.m_queue, finalizationChunk.m_commandBuffer);

	// Destroy the staging buffer
	m_logicalDevice.destroyBuffer(stagingBuffer.m_buffer);
	m_logicalDevice.freeMemory(stagingBuffer.m_bufferMemory);

	// Make a staging buffer for indices
	inputChunk.m_size = sizeof(uint32_t) * m_indexLump.size();
	inputChunk.m_usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	stagingBuffer = vkUtil::CreateBuffer(inputChunk);

	// Fill it with index data
	memoryLocation = m_logicalDevice.mapMemory(stagingBuffer.m_bufferMemory, 0, inputChunk.m_size);
	memcpy(memoryLocation, m_indexLump.data(), inputChunk.m_size);
	m_logicalDevice.unmapMemory(stagingBuffer.m_bufferMemory);

	// Make the index buffer
	inputChunk.m_usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
	inputChunk.m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	m_indexBuffer = vkUtil::CreateBuffer(inputChunk);

	// Fill it
	vkUtil::CopyBuffer(stagingBuffer, m_indexBuffer, inputChunk.m_size, finalizationChunk.m_queue, finalizationChunk.m_commandBuffer);

	// Destroy the staging buffer
	m_logicalDevice.destroyBuffer(stagingBuffer.m_buffer);
	m_logicalDevice.freeMemory(stagingBuffer.m_bufferMemory);
}

VertexManager::~VertexManager() 
{
	m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
	m_logicalDevice.freeMemory(m_vertexBuffer.m_bufferMemory);

	m_logicalDevice.destroyBuffer(m_indexBuffer.m_buffer);
	m_logicalDevice.freeMemory(m_indexBuffer.m_bufferMemory);
}