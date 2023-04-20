#include "Application.h"

Application::Application(int width, int height) 
{
	BuildGlfwWindow(width, height);

	m_graphicsEngine = new Engine(width, height, m_window);
}

Application::~Application() 
{
	delete m_graphicsEngine;
}

void Application::BuildGlfwWindow(int width, int height)
{
	//initialize glfw
	glfwInit();

	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//resizing breaks the swapchain, we'll disable it for now
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//GLFWwindow* glfwCreateWindow (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
	m_window = glfwCreateWindow(width, height, "LearnVulkanEngine", nullptr, nullptr);

	if (!m_window)
		throw std::runtime_error("GLFW window creation failed\n");
}

void Application::Run() 
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		m_graphicsEngine->Render();
		CalculateFrameRate();
	}
}

void Application::CalculateFrameRate() 
{
	m_currentTime = glfwGetTime();
	double deltaTime = m_currentTime - m_lastTime;

	if (deltaTime >= 1) 
	{
		int framerate{ std::max(1, int(m_numFrames / deltaTime)) };
		std::stringstream title;
		title << "Running at " << framerate << " fps.";
		glfwSetWindowTitle(m_window, title.str().c_str());
		m_lastTime = m_currentTime;
		m_numFrames = -1;
		m_frameTime = float(1000.0 / framerate);
	}

	++m_numFrames;
}


