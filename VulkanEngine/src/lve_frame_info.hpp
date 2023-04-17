#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace lve 
{
	#define MAX_LIGHTS 10

	struct PointLight 
	{
		glm::vec4 position{};  // ignore w
		glm::vec4 color{};     // w is intensity
	};

	struct GlobalUBO 
	{
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

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