#pragma once
#include "Config.h"

class Scene
{
public:
	Scene();
	
	std::vector<glm::vec3> m_trianglePositions;

	std::vector<glm::vec3> m_squarePositions;

	std::vector<glm::vec3> m_starPositions;
};