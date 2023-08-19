#pragma once
#include "Config.h"
#include "Memory.h"

struct FinalizationChunk
{
	vk::Device m_logicalDevice;
	vk::PhysicalDevice m_physicalDevice;
	vk::Queue m_queue;
	vk::CommandBuffer m_commandBuffer;
};

class VertexManager 
{
public:
	VertexManager();
	~VertexManager();
	void Consume(MeshTypes type, std::vector<float>& vertexData, std::vector<uint32_t>& indexData);
	void Finalize(const FinalizationChunk& finalizationChunk);
	Buffer m_vertexBuffer, m_indexBuffer;
	std::unordered_map<MeshTypes, int> m_firstIndices;
	std::unordered_map<MeshTypes, int> m_indexCounts;
private:
	int m_indexOffset;
	vk::Device m_logicalDevice;
	std::vector<float> m_vertexLump;
	std::vector<uint32_t> m_indexLump;
};