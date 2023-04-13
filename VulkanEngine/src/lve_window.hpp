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
		bool WasWindowResized() { return m_framebufferResized; }
		void ResetWindowResizedFlag() { m_framebufferResized = false; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		void InitWindow();

		int m_width;
		int m_height;
		bool m_framebufferResized = false;

		std::string m_windowName;
		GLFWwindow* m_window;
	};
}