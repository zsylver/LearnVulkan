#include "first_app.hpp"

#include <array>
#include <stdexcept>
#include <cassert>
namespace lve
{
	FirstApp::FirstApp()
	{
		LoadModels();

		CreatePipelineLayout();
		RecreateSwapChain();
		CreateCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(m_lveDevice.Device(), m_pipelineLayout, nullptr);
	}

	void lve::FirstApp::Run()
	{
		while (!m_lveWindow.ShouldClose())
		{
			glfwPollEvents(); // Input
			DrawFrame();
		}

		vkDeviceWaitIdle(m_lveDevice.Device());
	}

	void FirstApp::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(m_lveDevice.Device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void FirstApp::CreatePipeline()
	{
		assert(m_lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.m_renderPass = m_lveSwapChain->GetRenderPass();
		pipelineConfig.m_pipelineLayout = m_pipelineLayout;
		m_lvePipeline = std::make_unique<LvePipeline>(
			m_lveDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::RecreateSwapChain()
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
			m_lveSwapChain = std::make_unique<LveSwapChain>(m_lveDevice, extent, std::move(m_lveSwapChain));
			if (m_lveSwapChain->ImageCount() != m_commandBuffers.size())
			{
				FreeCommandBuffers();
				CreateCommandBuffers();
			}
		}

		CreatePipeline();
	}

	void FirstApp::CreateCommandBuffers()
	{
		m_commandBuffers.resize(m_lveSwapChain->ImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_lveDevice.GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_lveDevice.Device(), &allocInfo, m_commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void FirstApp::RecordCommandBuffers(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_lveSwapChain->GetRenderPass();
		renderPassInfo.framebuffer = m_lveSwapChain->GetFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_lveSwapChain->GetSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_lveSwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(m_lveSwapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_lveSwapChain->GetSwapChainExtent() };
		vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

		m_lvePipeline->Bind(m_commandBuffers[imageIndex]);
		m_lveModel->Bind(m_commandBuffers[imageIndex]);
		m_lveModel->Draw(m_commandBuffers[imageIndex]);

		vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void FirstApp::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_lveDevice.Device(),
			m_lveDevice.GetCommandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data());
		m_commandBuffers.clear();
	}

	void FirstApp::DrawFrame()
	{
		uint32_t imageIndex;
		auto result = m_lveSwapChain->AcquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		RecordCommandBuffers(imageIndex);
		result = m_lveSwapChain->SubmitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_lveWindow.WasWindowResized())
		{
			m_lveWindow.ResetWindowResizedFlag();
			RecreateSwapChain();
			return;
		}	
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void FirstApp::LoadModels()
	{
		std::vector<LveModel::Vertex> vertices;
		//{
		//	{{0.0f, -0.5f}, {1, 0, 0}},
		//	{{0.5f, 0.5f}, {0, 1, 0}},
		//	{{-0.5f, 0.5f}, {0, 0, 1}}
		//};
		Sierpinski(vertices, 5, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, -0.5f });
		m_lveModel = std::make_unique<LveModel>(m_lveDevice, vertices);
	}

	void FirstApp::Sierpinski(std::vector<LveModel::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top)
	{
		if (depth <= 0) 
		{
			vertices.push_back({ top, { 1, 0, 0} });
			vertices.push_back({ right, { 0, 1, 0} });
			vertices.push_back({ left, { 0, 0, 1} });
		}
		else 
		{
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			Sierpinski(vertices, depth - 1, left, leftRight, leftTop);
			Sierpinski(vertices, depth - 1, leftRight, right, rightTop);
			Sierpinski(vertices, depth - 1, leftTop, rightTop, top);
		}
	}
}

