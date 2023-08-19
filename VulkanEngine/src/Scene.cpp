#include "Scene.h"

Scene::Scene()
{
	m_positions.insert({ MeshTypes::GROUND, {} });
	m_positions.insert({ MeshTypes::GIRL, {} });
	m_positions.insert({ MeshTypes::SKULL, {} });
	m_positions.insert({ MeshTypes::ROOM, {} });

	m_positions[MeshTypes::GROUND].push_back(glm::vec3(10.f, 0.f, 0.f));
	m_positions[MeshTypes::GIRL].push_back(glm::vec3(14.f, 0.f, 0.f));
	//m_positions[MeshTypes::ROOM].push_back(glm::vec3(5.f, 0.f, 0.f));
	m_positions[MeshTypes::SKULL].push_back(glm::vec3(10.f, -5.f, 0.f));
	m_positions[MeshTypes::SKULL].push_back(glm::vec3(10.f, 5.f, 0.f));
}
