#pragma once
#include "Config.h"

namespace vkUtil 
{

	/**
		Read a file.
		\param filename a string representing the path to the file
		\param debug whether the system is running in debug mode
		\returns the contents as a vector of raw binary characters
	*/
	std::vector<char> readFile(std::string filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) 
		{
			std::cout << "Failed to load \"" << filename << "\"" << std::endl;
		}

		size_t filesize{ static_cast<size_t>(file.tellg()) };

		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();
		return buffer;
	}


	/**
		Make a shader module.
		\param filename a string holding the filepath to the spir-v file.
		\param device the logical device
		\param debug whether the system is running in debug mode
		\returns the created shader module
	*/
	vk::ShaderModule CreateModule(std::string filename, vk::Device device)
	{
		std::vector<char> sourceCode = readFile(filename);
		vk::ShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.flags = vk::ShaderModuleCreateFlags();
		moduleInfo.codeSize = sourceCode.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

		try 
		{
			return device.createShaderModule(moduleInfo);
		}
		catch (vk::SystemError err) 
		{
			throw std::runtime_error("Error: Failed to create shader module!\n");
		}
	}
}