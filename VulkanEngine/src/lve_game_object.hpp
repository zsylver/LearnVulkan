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

		glm::mat4 mat4() 
		{
			const float c3 = glm::cos(m_rotation.z);
			const float s3 = glm::sin(m_rotation.z);
			const float c2 = glm::cos(m_rotation.x);
			const float s2 = glm::sin(m_rotation.x);
			const float c1 = glm::cos(m_rotation.y);
			const float s1 = glm::sin(m_rotation.y);
			return glm::mat4
			{
				{
					m_scale.x * (c1 * c3 + s1 * s2 * s3),
					m_scale.x * (c2 * s3),
					m_scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					m_scale.y * (c3 * s1 * s2 - c1 * s3),
					m_scale.y * (c2 * c3),
					m_scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					m_scale.z * (c2 * s1),
					m_scale.z * (-s2),
					m_scale.z * (c1 * c2),
					0.0f,
				},
				{ m_translation.x, m_translation.y, m_translation.z, 1.0f }
			};
		}
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