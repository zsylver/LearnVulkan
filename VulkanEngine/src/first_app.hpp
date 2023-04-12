#pragma once

#include "lve_pipeline.hpp"
#include "lve_window.hpp"
#include "lve_swap_chain.hpp"
#include "lve_device.hpp"

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

		LveWindow m_lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
		LveDevice m_lveDevice{ m_lveWindow };
		LveSwapChain m_lveSwapChain{ m_lveDevice, m_lveWindow.GetExtent() };
		std::unique_ptr<LvePipeline> m_lvePipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
}	// namespace lve