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
	void Consume(MeshTypes type, std::vector<float> vertexData);
	void Finalize(const FinalizationChunk& finalizationChunk);
	Buffer m_vertexBuffer;
	std::unordered_map<MeshTypes, int> m_offsets;
	std::unordered_map<MeshTypes, int> m_sizes;
private:
	int m_offset;
	vk::Device m_logicalDevice;
	std::vector<float> m_lump;
};