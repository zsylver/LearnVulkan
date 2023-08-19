#pragma once
#include "Config.h"

class Scene
{
public:
	Scene();
	
	std::unordered_map<MeshTypes, std::vector<glm::vec3>> m_positions;
};