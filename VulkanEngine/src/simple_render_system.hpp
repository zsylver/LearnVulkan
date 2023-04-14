#pragma once

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve 
{
	class SimpleRenderSystem 
	{
	public:
		SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects);

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass renderPass);

		LveDevice& m_lveDevice;

		std::unique_ptr<LvePipeline> m_lvePipeline;
		VkPipelineLayout m_pipelineLayout;
	};
}  // namespace lve