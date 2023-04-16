#pragma once

#include "lve_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace lve 
{
	struct FrameInfo 
	{
		int m_frameIndex;
		float m_frameTime;
		VkCommandBuffer m_commandBuffer;
		LveCamera& m_camera;
		VkDescriptorSet m_globalDescriptorSet;
	};
}  // namespace lve