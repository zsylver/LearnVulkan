#include "first_app.hpp"
#include "simple_render_system.hpp"

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
	}

	FirstApp::~FirstApp() {}

	void lve::FirstApp::Run()
	{
		SimpleRenderSystem simpleRenderSystem{ m_lveDevice, m_lveRenderer.GetSwapChainRenderPass() };

		while (!m_lveWindow.ShouldClose())
		{
			glfwPollEvents(); // Input
			if (auto commandBuffer = m_lveRenderer.BeginFrame()) {
				m_lveRenderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects);
				m_lveRenderer.EndSwapChainRenderPass(commandBuffer);
				m_lveRenderer.EndFrame();
			}
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
		//triangle.m_transform2D.m_rotation = 0.25f * glm::two_pi<float>();

		//m_gameObjects.push_back(std::move(triangle));
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

