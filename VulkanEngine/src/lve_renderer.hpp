#pragma once

#include "lve_pipeline.hpp"
#include "lve_window.hpp"
#include "lve_swap_chain.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace lve
{
	class LveRenderer
	{
	public:
		LveRenderer(LveWindow &window, LveDevice &device);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;

		VkRenderPass GetSwapChainRenderPass() const { return m_lveSwapChain->GetRenderPass(); }
		float GetAspectRatio() const { return m_lveSwapChain->ExtentAspectRatio(); }
		int GetFrameIndex() const { return m_currentFrameIndex; }
		bool IsFrameInProgress() const { return m_isFrameStarted; }

		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_commandBuffers[m_currentImageIndex];
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
	private:
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();

		LveWindow& m_lveWindow;
		LveDevice& m_lveDevice;
		std::unique_ptr<LveSwapChain> m_lveSwapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;
	};
}	// namespace lve