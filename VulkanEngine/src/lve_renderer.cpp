#include "lve_renderer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>

namespace lve
{
	LveRenderer::LveRenderer(LveWindow &window, LveDevice& device)
		: m_lveWindow{window}, m_lveDevice{device}
	{
		RecreateSwapChain();
		CreateCommandBuffers();
	}

	LveRenderer::~LveRenderer()
	{
		FreeCommandBuffers();
	}

	void LveRenderer::RecreateSwapChain()
	{
		auto extent = m_lveWindow.GetExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_lveWindow.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_lveDevice.Device());

		if (m_lveSwapChain == nullptr)
		{
			m_lveSwapChain = std::make_unique<LveSwapChain>(m_lveDevice, extent);
		}
		else
		{
			std::shared_ptr<LveSwapChain> oldSwapChain = std::move(m_lveSwapChain);
			m_lveSwapChain = std::make_unique<LveSwapChain>(m_lveDevice, extent, oldSwapChain);

			if (!oldSwapChain->CompareSwapFormats(*m_lveSwapChain.get())) 
			{
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void LveRenderer::CreateCommandBuffers()
	{
		m_commandBuffers.resize(m_lveSwapChain->ImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_lveDevice.GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_lveDevice.Device(), &allocInfo, 
			m_commandBuffers.data()) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void LveRenderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_lveDevice.Device(),
			m_lveDevice.GetCommandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data());
		m_commandBuffers.clear();
	}

	VkCommandBuffer LveRenderer::BeginFrame()
	{
		assert(!m_isFrameStarted && "Can't call BeginFrame while already in progress");
		
		auto result = m_lveSwapChain->AcquireNextImage(&m_currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		
		m_isFrameStarted = true;

		auto commandBuffer = GetCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void LveRenderer::EndFrame()
	{
		assert(m_isFrameStarted && "Can't call EndFrame while frame is not in progress");
		auto commandBuffer = GetCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer");
		}

		auto result = m_lveSwapChain->SubmitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_lveWindow.WasWindowResized())
		{
			m_lveWindow.ResetWindowResizedFlag();
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_isFrameStarted = false;
		m_currentFrameIndex = (m_currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void LveRenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_lveSwapChain->GetRenderPass();
		renderPassInfo.framebuffer = m_lveSwapChain->GetFrameBuffer(m_currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_lveSwapChain->GetSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_lveSwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(m_lveSwapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_lveSwapChain->GetSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void LveRenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Can't call EndSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}


