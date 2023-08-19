#include "TriangleMesh.h"

TriangleMesh::TriangleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice)
{
	m_logicalDevice = logicalDevice;

	std::vector<float> vertices
	{
		{
			 0.0f, -0.05f, 0.0f, 1.0f, 0.0f,
			 0.05f, 0.05f, 0.0f, 1.0f, 0.0f,
			-0.05f, 0.05f, 0.0f, 1.0f, 0.0f
		} 
	};

	BufferInputChunk inputChunk;
	inputChunk.m_logicalDevice = logicalDevice;
	inputChunk.m_physicalDevice = physicalDevice;
	inputChunk.m_size = sizeof(float) * vertices.size();
	inputChunk.m_usage = vk::BufferUsageFlagBits::eVertexBuffer;

	m_vertexBuffer = vkUtil::CreateBuffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(m_vertexBuffer.m_bufferMemory, 0, inputChunk.m_size);
	memcpy(memoryLocation, vertices.data(), inputChunk.m_size);
	logicalDevice.unmapMemory(m_vertexBuffer.m_bufferMemory);
}

TriangleMesh::~TriangleMesh() 
{
	m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
	m_logicalDevice.freeMemory(m_vertexBuffer.m_bufferMemory);
}
