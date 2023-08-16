#include "Scene.h"

Scene::Scene()
{
	for (float x = -1.f; x < 1.f; x += 0.2f)
		for (float y = -1.f; y < 1.f; y += 0.2f)
			trianglePositions.push_back(glm::vec3(x, y, 0.f));
}
