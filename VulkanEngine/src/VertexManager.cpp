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

void VertexManager::Finalize(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice) 
{
	m_logicalDevice = logicalDevice;

	BufferInputChunk inputChunk;
	inputChunk.m_logicalDevice = logicalDevice;
	inputChunk.m_physicalDevice = physicalDevice;
	inputChunk.m_size = sizeof(float) * m_lump.size();
	inputChunk.m_usage = vk::BufferUsageFlagBits::eVertexBuffer;

	m_vertexBuffer = vkUtil::CreateBuffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(m_vertexBuffer.m_bufferMemory, 0, inputChunk.m_size);
	memcpy(memoryLocation, m_lump.data(), inputChunk.m_size);
	logicalDevice.unmapMemory(m_vertexBuffer.m_bufferMemory);
}

VertexManager::~VertexManager() 
{
	m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
	m_logicalDevice.freeMemory(m_vertexBuffer.m_bufferMemory);

}