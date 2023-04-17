#pragma once

#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"

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
		void LoadGameObjects();

		LveWindow m_lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
		LveDevice m_lveDevice{ m_lveWindow };
		LveRenderer m_lveRenderer{ m_lveWindow, m_lveDevice };

		// note: order of declarations matters
		std::unique_ptr<LveDescriptorPool> m_globalPool{};
		LveGameObject::Map m_gameObjects;
	};
}	// namespace lve