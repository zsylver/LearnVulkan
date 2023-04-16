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

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<LveModel> CreateCubeModel(LveDevice& device, glm::vec3 offset)
	{
		LveModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}

		modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		return std::make_unique<LveModel>(device, modelBuilder);
	}

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
		std::shared_ptr<LveModel> lveModel = CreateCubeModel(m_lveDevice, { .0f, .0f, .0f });
		auto cube = LveGameObject::CreateGameObject();
		cube.m_model = lveModel;
		cube.m_transform.m_translation = { .0f, .0f, 2.5f };
		cube.m_transform.m_scale = { .5f, .5f, .5f };
		m_gameObjects.push_back(std::move(cube));
	}

}

