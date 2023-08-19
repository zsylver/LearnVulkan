#include "ObjMesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

vkMesh::ObjMesh::ObjMesh(glm::mat4 preTransform, const char* objFilepath, const char* mtlFilepath)
{
	//tinyobj::attrib_t attrib;
	//std::vector<tinyobj::shape_t> shapes;
	//std::vector<tinyobj::material_t> materials;
	//std::string warn, err;

	//if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFilepath)) 
	//{
	//	throw std::runtime_error(warn + err);
	//}

	//for (const auto& shape : shapes) 
	//{
	//	for (const auto& index : shape.mesh.indices) 
	//	{
	//		// Positions
	//		m_vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
	//		m_vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
	//		m_vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

	//		// UV
	//		m_vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
	//		m_vertices.push_back(1.f - attrib.texcoords[2 * index.texcoord_index + 1]);

	//		// Color (RGB)
	//		m_vertices.push_back(1.f);
	//		m_vertices.push_back(1.f);
	//		m_vertices.push_back(1.f);

	//		// Normal
	//		m_vertices.push_back(attrib.normals[3 * index.normal_index + 0]);
	//		m_vertices.push_back(attrib.normals[3 * index.normal_index + 1]);
	//		m_vertices.push_back(attrib.normals[3 * index.normal_index + 2]);

	//		m_indices.push_back(index.vertex_index);
	//	}
	//}

	m_preTransform = preTransform;

	std::ifstream file;
	std::string line;
	std::string materialName;
	std::vector<std::string> words;

	if (mtlFilepath != "none")
	{
		file.open(mtlFilepath);

		while (std::getline(file, line))
		{
			words = Split(line, " ");

			if (!words[0].compare("newmtl"))
			{
				materialName = words[1];
			}

			if (!words[0].compare("Kd"))
			{
				m_brushColor = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
				m_colorLookup.insert({ materialName, m_brushColor });
			}
		}

		file.close();
	}

	file.open(objFilepath);

	while (std::getline(file, line)) 
	{
		words = Split(line, " ");

		if (!words[0].compare("v")) 
		{
			ReadVertexData(words);
		}
		if (!words[0].compare("vt")) 
		{
			ReadTexCoordData(words);
		}
		if (!words[0].compare("vn")) 
		{
			ReadNormalData(words);
		}
		if (!words[0].compare("usemtl")) 
		{
			if (m_colorLookup.contains(words[1]))
			{
				m_brushColor = m_colorLookup[words[1]];
			}
			else 
			{
				m_brushColor = glm::vec3(1.0f);
			}
		}
		if (!words[0].compare("f")) 
		{
			ReadFaceData(words);
		}
	}
}

void vkMesh::ObjMesh::ReadVertexData(const std::vector<std::string>& words)
{
	glm::vec4 newVertex = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f);
	glm::vec3 transformedVertex = glm::vec3(m_preTransform * newVertex);
	m_v.push_back(transformedVertex);
}

void vkMesh::ObjMesh::ReadTexCoordData(const std::vector<std::string>& words)
{
	glm::vec2 newVertex = glm::vec2(std::stof(words[1]), std::stof(words[2]));
	m_vt.push_back(newVertex);
}

void vkMesh::ObjMesh::ReadNormalData(const std::vector<std::string>& words)
{
	glm::vec4 newVertex = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f);
	glm::vec3 transformedVertex = glm::vec3(m_preTransform * newVertex);
	m_vn.push_back(transformedVertex);
}

void vkMesh::ObjMesh::ReadFaceData(const std::vector<std::string>& words)
{
	size_t triangleCount = words.size() - 3;

	for (int i = 0; i < triangleCount; ++i)
	{
		ReadCorner(words[1]);
		ReadCorner(words[2 + i]);
		ReadCorner(words[3 + i]);
	}
}

void vkMesh::ObjMesh::ReadCorner(const std::string& vertexDescription)
{
	if (m_history.contains(vertexDescription))
	{
		m_indices.push_back(m_history[vertexDescription]);
		return;
	}

	uint32_t index = static_cast<uint32_t>(m_history.size());
	m_history.insert({ vertexDescription, index });
	m_indices.push_back(index);

	std::vector<std::string> v_vt_vn = Split(vertexDescription, "/");

	//Position
	glm::vec3 pos = m_v[std::stol(v_vt_vn[0]) - 1];
	m_vertices.push_back(pos[0]);
	m_vertices.push_back(pos[1]);
	m_vertices.push_back(pos[2]);

	//Color
	m_vertices.push_back(m_brushColor.r);
	m_vertices.push_back(m_brushColor.g);
	m_vertices.push_back(m_brushColor.b);

	//TexCoord
	glm::vec2 texcoord = glm::vec2(0.0f, 0.0f);
	if (v_vt_vn.size() == 3 && v_vt_vn[1].size() > 0) 
	{
		texcoord = m_vt[std::stol(v_vt_vn[1]) - 1];
	}
	m_vertices.push_back(texcoord[0]);
	m_vertices.push_back(texcoord[1]);

	//Normal
	glm::vec3 normal = m_vn[std::stol(v_vt_vn[2]) - 1];
	m_vertices.push_back(normal[0]);
	m_vertices.push_back(normal[1]);
	m_vertices.push_back(normal[2]);
}
