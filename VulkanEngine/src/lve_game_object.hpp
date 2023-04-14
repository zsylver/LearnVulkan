#pragma once

#include "lve_model.hpp"

// std
#include <memory>

namespace lve
{
	struct Transform2DComponent
	{
		glm::vec2 m_translation{};
		glm::vec2 m_scale{ 1.0f, 1.0f };
		float m_rotation{};

		glm::mat2 mat2() 
		{
			glm::mat2 rotMtx{ { glm::cos(m_rotation), glm::sin(m_rotation) },
							 { -glm::sin(m_rotation), glm::cos(m_rotation) } };
			glm::mat2 scaleMtx{ { m_scale.x, 0.0f }, { 0.0f, m_scale.y } };
			return rotMtx * scaleMtx;
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
		Transform2DComponent m_transform2D;

	private:
		LveGameObject(id_t objID) : m_ID{ objID } {}

		id_t m_ID;
	};
}