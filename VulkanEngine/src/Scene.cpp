#include "Scene.h"

Scene::Scene()
{
	float x = 0.3f;
	for (float z = -1.0f; z <= 1.0f; z += 0.2f) 
	{
		for (float y = -1.0f; y <= 1.0f; y += 0.2f) 
		{
			m_trianglePositions.push_back(glm::vec3(x, y, z));
		}
	}

	x = 0.0f;
	for (float z = -1.0f; z <= 1.0f; z += 0.2f) 
	{
		for (float y = -1.0f; y < 1.0f; y += 0.2f) 
		{
			m_squarePositions.push_back(glm::vec3(x, y, z));
		}
	}

	x = -0.3f;
	for (float z = -1.0f; z <= 1.0f; z += 0.2f) 
	{
		for (float y = -1.0f; y < 1.0f; y += 0.2f) 
		{
			m_starPositions.push_back(glm::vec3(x, y, z));
		}
	}
}
