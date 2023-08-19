#pragma once
#include "Config.h"

namespace vkInit
{
	/**
		Describes the bindings of a descriptor set layout
	*/
	struct DescriptorSetLayoutData
	{
		int m_count;
		std::vector<int> m_indices;
		std::vector<vk::DescriptorType> m_types;
		std::vector<int> m_counts;
		std::vector<vk::ShaderStageFlags> m_stages;
	};


	/**
		Make a descriptor set layout from the given descriptions

		\param device the logical device
		\param bindings	a struct describing the bindings used in the shader
		\returns the created descriptor set layout
	*/
	vk::DescriptorSetLayout CreateDescriptorSetLayout(vk::Device device, const DescriptorSetLayoutData& bindings);

	vk::DescriptorPool CreateDescriptorPool(vk::Device device, uint32_t size, const DescriptorSetLayoutData& bindings);

	vk::DescriptorSet AllocateDescriptorSet(vk::Device device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout layout);
}