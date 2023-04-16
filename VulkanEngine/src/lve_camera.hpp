#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve 
{
    class LveCamera 
    {
    public:
        void SetOrthographicProjection(
            float left, float right, float top, float bottom, float near, float far);

        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

        const glm::mat4& GetProjection() const { return m_projectionMatrix; }

    private:
        glm::mat4 m_projectionMatrix{ 1.f };
    };
}  // namespace lve