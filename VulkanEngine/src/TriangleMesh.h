#pragma once
#include "Config.h"
#include "Memory.h"

class TriangleMesh
{
public:
	TriangleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);
	~TriangleMesh();
	Buffer m_vertexBuffer;
private:
	vk::Device m_logicalDevice;
};