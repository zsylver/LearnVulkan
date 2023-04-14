#include "first_app.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>
#include <cassert>
namespace lve
{

	struct SimplePushConstantData
	{
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp()
	{
		LoadGameObjects();

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

	void FirstApp::LoadGameObjects()
	{
		std::vector<LveModel::Vertex> vertices;
		//{
		//	{{0.0f, -0.5f}, {1, 0, 0}},
		//	{{0.5f, 0.5f}, {0, 1, 0}},
		//	{{-0.5f, 0.5f}, {0, 0, 1}}
		//};
		Sierpinski(vertices, 5, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, -0.5f });
		auto lveModel = std::make_shared<LveModel>(m_lveDevice, vertices);

		auto triangle = LveGameObject::CreateGameObject();

		std::vector<glm::vec3> colors
		{
			{1.0f, 0.7f, 0.73f},
			{1.0f, 0.87f, 0.73f},
			{1.0f, 1.0f, 0.73f},
			{0.73f, 1.0f, 0.8f},
			{0.73, 0.88f, 1.0f}
		};

		// applying gamma
		for (auto& color : colors) {
			color = glm::pow(color, glm::vec3{ 2.2f });
		}

		for (int i = 0; i < 40; i++) {
			auto triangle = LveGameObject::CreateGameObject();
			triangle.m_model = lveModel;
			triangle.m_transform2D.m_scale = glm::vec2(.5f) + i * 0.025f;
			triangle.m_transform2D.m_rotation = i * glm::pi<float>() * .025f;
			triangle.m_color = colors[i % colors.size()];
			m_gameObjects.push_back(std::move(triangle));
		}

		//triangle.m_model = lveModel;
		//triangle.m_color = { 0.1f, 0.8f, 0.1f };
		//triangle.m_transform2D.m_translation.x = 0.2f;
		//triangle.m_transform2D.m_scale = { 2.0f, 0.5f };
		//triangle.m_transform2D.rotation = 0.25f * glm::two_pi<float>();

		//m_gameObjects.push_back(std::move(triangle));
	}

	void FirstApp::RenderGameObjects(VkCommandBuffer cmdBuffer)
	{
		// update
		int i = 0;
		for (auto& obj : m_gameObjects)
		{
			i += 1;
			obj.m_transform2D.m_rotation =
				glm::mod<float>(obj.m_transform2D.m_rotation + 0.00001f * i, 2.0f * glm::pi<float>());
		}

		// render
		m_lvePipeline->Bind(cmdBuffer);

		for (auto& obj : m_gameObjects)
		{
			SimplePushConstantData push{};
			push.offset = obj.m_transform2D.m_translation;
			push.color = obj.m_color;
			push.transform = obj.m_transform2D.mat2();

			vkCmdPushConstants(
				cmdBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.m_model->Bind(cmdBuffer);
			obj.m_model->Draw(cmdBuffer);
		}
	}

	void FirstApp::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
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
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
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

		RenderGameObjects(m_commandBuffers[imageIndex]);


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

