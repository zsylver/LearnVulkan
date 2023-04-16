#include "first_app.hpp"
#include "lve_buffer.hpp"
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
	struct GlobalUBO 
	{
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	FirstApp::FirstApp()
	{
		m_globalPool =
			LveDescriptorPool::Builder(m_lveDevice)
			.SetMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.Build();
		LoadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void lve::FirstApp::Run()
	{
		std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) 
		{
			uboBuffers[i] = std::make_unique<LveBuffer>(
				m_lveDevice,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->Map();
		}

		auto globalSetLayout =
			LveDescriptorSetLayout::Builder(m_lveDevice)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build();

		std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) 
		{
			auto bufferInfo = uboBuffers[i]->DescriptorInfo();
			LveDescriptorWriter(*globalSetLayout, *m_globalPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ m_lveDevice, m_lveRenderer.GetSwapChainRenderPass(), globalSetLayout->GetDescriptorSetLayout() };
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
			float frameTime =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Movement Input
			cameraController.MoveInPlaneXZ(m_lveWindow.GetGLFWwindow(), frameTime, viewerObject);
			camera.SetViewYXZ(viewerObject.m_transform.m_translation, viewerObject.m_transform.m_rotation);

			float aspect = m_lveRenderer.GetAspectRatio();
			// camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = m_lveRenderer.BeginFrame()) 
			{
				int frameIndex = m_lveRenderer.GetFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex] };

				// update
				GlobalUBO ubo{};
				ubo.projectionView = camera.GetProjection() * camera.GetView();
				uboBuffers[frameIndex]->WriteToBuffer(&ubo);
				uboBuffers[frameIndex]->Flush();

				// render
				m_lveRenderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(frameInfo, m_gameObjects);
				m_lveRenderer.EndSwapChainRenderPass(commandBuffer);
				m_lveRenderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(m_lveDevice.Device());
	}

	void FirstApp::LoadGameObjects()
	{
		std::shared_ptr<LveModel> lveModel =
			LveModel::CreateModelFromFile(m_lveDevice, "models/flat_vase.obj");
		auto flatVase = LveGameObject::CreateGameObject();
		flatVase.m_model = lveModel;
		flatVase.m_transform.m_translation = { -.5f, .5f, 2.5f };
		flatVase.m_transform.m_scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.push_back(std::move(flatVase));

		lveModel = LveModel::CreateModelFromFile(m_lveDevice, "models/smooth_vase.obj");
		auto smoothVase = LveGameObject::CreateGameObject();
		smoothVase.m_model = lveModel;
		smoothVase.m_transform.m_translation = { .5f, .5f, 2.5f };
		smoothVase.m_transform.m_scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.push_back(std::move(smoothVase));
	}

}

