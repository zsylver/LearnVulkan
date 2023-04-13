#pragma once

#include "lve_pipeline.hpp"
#include "lve_window.hpp"
#include "lve_swap_chain.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
	class FirstApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void Run();
	private:
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void DrawFrame();
		void RecreateSwapChain();
		void RecordCommandBuffers(int imageIndex);
		void FreeCommandBuffers();

		LveWindow m_lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
		LveDevice m_lveDevice{ m_lveWindow };
		std::unique_ptr<LveSwapChain> m_lveSwapChain;
		std::unique_ptr<LvePipeline> m_lvePipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkCommandBuffer> m_commandBuffers;

		void LoadModels();
		void Sierpinski(
			std::vector<LveModel::Vertex>& vertices,
			int depth,
			glm::vec2 left,
			glm::vec2 right,
			glm::vec2 top);

		std::unique_ptr<LveModel> m_lveModel;
	};
}	// namespace lve