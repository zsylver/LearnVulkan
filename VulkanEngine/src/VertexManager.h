#pragma once
#include "Config.h"
#include "Memory.h"

class VertexManager 
{
public:
	VertexManager();
	~VertexManager();
	void Consume(MeshTypes type, std::vector<float> vertexData);
	void Finalize(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);
	Buffer m_vertexBuffer;
	std::unordered_map<MeshTypes, int> m_offsets;
	std::unordered_map<MeshTypes, int> m_sizes;
private:
	int m_offset;
	vk::Device m_logicalDevice;
	std::vector<float> m_lump;
};