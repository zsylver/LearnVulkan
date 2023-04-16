#pragma once

#include "lve_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace lve
{
	struct TransformComponent
	{
		glm::vec3 m_translation{};
		glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 m_rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix

		glm::mat4 mat4();

		glm::mat3 NormalMatrix();
	};

	class LveGameObject
	{
	public:
		using id_t = unsigned int;

		static LveGameObject CreateGameObject()
		{
			static id_t currentID = 0;
			return LveGameObject{ currentID++ };
		}

		LveGameObject(const LveGameObject&) = delete;
		LveGameObject& operator=(const LveGameObject&) = delete;
		LveGameObject(LveGameObject&&) = default;
		LveGameObject& operator=(LveGameObject&&) = default;

		id_t GetID() { return m_ID; }

		std::shared_ptr<LveModel> m_model{};
		glm::vec3  m_color{};
		TransformComponent m_transform;

	private:
		LveGameObject(id_t objID) : m_ID{ objID } {}

		id_t m_ID;
	};
}