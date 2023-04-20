#pragma once
#include "Config.h"
#include "Engine.h"

class Application 
{
public:
	Application(int width, int height);
	~Application();
	void Run();

private:
	Engine* m_graphicsEngine;
	GLFWwindow* m_window;

	double m_lastTime, m_currentTime;
	int m_numFrames;
	float m_frameTime;

	void BuildGlfwWindow(int width, int height);

	void CalculateFrameRate();
};