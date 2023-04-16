#include "first_app.hpp"
#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
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
		LveCamera camera{};
		// camera.SetViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
		camera.SetViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = LveGameObject::CreateGameObject();
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!m_lveWindow.ShouldClose())
		{
			glfwPollEvents(); // Input

			auto newTime = std::chrono::high_resolution_clock::now();
			float dt =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Movement Input
			cameraController.MoveInPlaneXZ(m_lveWindow.GetGLFWwindow(), dt, viewerObject);
			camera.SetViewYXZ(viewerObject.m_transform.m_translation, viewerObject.m_transform.m_rotation);

			float aspect = m_lveRenderer.GetAspectRatio();
			// camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = m_lveRenderer.BeginFrame()) 
			{
				m_lveRenderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects, camera);
				m_lveRenderer.EndSwapChainRenderPass(commandBuffer);
				m_lveRenderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(m_lveDevice.Device());
	}

	void FirstApp::LoadGameObjects()
	{
		std::shared_ptr<LveModel> lveModel =
			LveModel::CreateModelFromFile(m_lveDevice, "models/smooth_vase.obj");
		auto gameObj = LveGameObject::CreateGameObject();
		gameObj.m_model = lveModel;
		gameObj.m_transform.m_translation = { .0f, .0f, 2.5f };
		gameObj.m_transform.m_scale = glm::vec3(3.f);
		m_gameObjects.push_back(std::move(gameObj));
	}

}

