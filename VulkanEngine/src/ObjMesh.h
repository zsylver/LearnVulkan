#pragma once
#include "Config.h"

namespace vkMesh 
{
	class ObjMesh
	{
	public:
		std::vector<float> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<glm::vec3> m_v, m_vn;
		std::vector<glm::vec2> m_vt;
		std::unordered_map<std::string, uint32_t> m_history;
		std::unordered_map<std::string, glm::vec3> m_colorLookup;
		glm::vec3 m_brushColor;
		glm::mat4 m_preTransform;

		ObjMesh(glm::mat4 preTransform, const char* objFilepath, const char* mtlFilepath = "none");

		void ReadVertexData(const std::vector<std::string>& words); // read v

		void ReadTexCoordData(const std::vector<std::string>& words); // read vt

		void ReadNormalData(const std::vector<std::string>& words); // read vn

		void ReadFaceData(const std::vector<std::string>& words); // read f

		void ReadCorner(const std::string& vertex_description);
	};
}