#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace lve
{
	class LveWindow
	{
	public:
		LveWindow(int w, int h, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(m_window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		void InitWindow();

		const int m_width;
		const int m_height;

		std::string m_windowName;
		GLFWwindow* m_window;
	};
}