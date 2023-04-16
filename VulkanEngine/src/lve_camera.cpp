#include "lve_camera.hpp"

// std
#include <cassert>
#include <limits>

namespace lve
{
	void LveCamera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) 
	{
		m_projectionMatrix = glm::mat4{ 1.0f };
		m_projectionMatrix[0][0] = 2.f / (right - left);
		m_projectionMatrix[1][1] = 2.f / (bottom - top);
		m_projectionMatrix[2][2] = 1.f / (far - near);
		m_projectionMatrix[3][0] = -(right + left) / (right - left);
		m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix[3][2] = -near / (far - near);
	}

	void LveCamera::SetPerspectiveProjection(float fovy, float aspect, float near, float far)
	{
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFovy = tan(fovy / 2.f);
		m_projectionMatrix = glm::mat4{ 0.0f };
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix[2][2] = far / (far - near);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(far * near) / (far - near);
	}
}
