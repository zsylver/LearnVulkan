#include "first_app.hpp"

#include <array>
#include <stdexcept>
namespace lve
{
	FirstApp::FirstApp()
	{
		LoadModels();

		CreatePipelineLayout();
		CreatePipeline();
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
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::DefaultPipelineConfigInfo(
			pipelineConfig,
			m_lveSwapChain.Width(),
			m_lveSwapChain.Height());
		pipelineConfig.m_renderPass = m_lveSwapChain.GetRenderPass();
		pipelineConfig.m_pipelineLayout = m_pipelineLayout;
		m_lvePipeline = std::make_unique<LvePipeline>(
			m_lveDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::CreateCommandBuffers()
	{
		m_commandBuffers.resize(m_lveSwapChain.ImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_lveDevice.GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_lveDevice.Device(), &allocInfo, m_commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (int i = 0; i < m_commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_lveSwapChain.GetRenderPass();
			renderPassInfo.framebuffer = m_lveSwapChain.GetFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_lveSwapChain.GetSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			m_lvePipeline->Bind(m_commandBuffers[i]);
			m_lveModel->Bind(m_commandBuffers[i]);
			m_lveModel->Draw(m_commandBuffers[i]);

			vkCmdEndRenderPass(m_commandBuffers[i]);
			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void FirstApp::DrawFrame()
	{
		uint32_t imageIndex;
		auto result = m_lveSwapChain.AcquireNextImage(&imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = m_lveSwapChain.SubmitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
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

