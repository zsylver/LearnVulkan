#include "Scene.h"

Scene::Scene()
{
	for (float x = -10.f; x < 11.f; x += 2.0f)
		for (float y = -10.f; y < 11.f; y += 2.0f)
			trianglePositions.push_back(glm::vec3(x, y, 0.f));
}
