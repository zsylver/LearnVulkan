#pragma once

//statically load vulkan library
#include <vulkan/vulkan.hpp>
/*
* including the prebuilt header from the lunarg sdk will load
* most functions, but not all.
*
* Functions can also be dynamically loaded, using the call
*
* PFN_vkVoidFunction vkGetInstanceProcAddr(
	VkInstance                                  instance,
	const char*                                 pName);
 or
 PFN_vkVoidFunction vkGetDeviceProcAddr(
	VkDevice                                    device,
	const char*                                 pName);
	We will look at this later, once we've created an instance and device.
*/

#include <GLFW/glfw3.h>
// std
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

struct BufferInputChunk
{
	size_t m_size;
	vk::BufferUsageFlags m_usage;
	vk::Device m_logicalDevice;
	vk::PhysicalDevice m_physicalDevice;
	vk::MemoryPropertyFlags m_memoryProperties;
};

struct Buffer
{
	vk::Buffer m_buffer;
	vk::DeviceMemory m_bufferMemory;
};

//--------- Assets -------------//
enum class MeshTypes 
{
	GROUND,
	GIRL,
	SKULL,
	ROOM
};

std::vector<std::string> Split(std::string line, std::string delimiter);