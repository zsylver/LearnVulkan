#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

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
		LveGameObject::Map& m_gameObjects;
	};
}  // namespace lve