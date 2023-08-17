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
	vk::DescriptorSetLayout CreateDescriptorSetLayout(vk::Device device, const DescriptorSetLayoutData& bindings)
	{

		/*
			Bindings describes a whole bunch of descriptor types, collect them all into a
			list of some kind.
		*/
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.reserve(bindings.m_count);

		for (int i = 0; i < bindings.m_count; i++)
		{
			/*
				typedef struct VkDescriptorSetLayoutBinding {
					uint32_t              binding;
					VkDescriptorType      descriptorType;
					uint32_t              descriptorCount;
					VkShaderStageFlags    stageFlags;
					const VkSampler*      pImmutableSamplers;
				} VkDescriptorSetLayoutBinding;
			*/

			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding			= bindings.m_indices[i];
			layoutBinding.descriptorType	= bindings.m_types[i];
			layoutBinding.descriptorCount	= bindings.m_counts[i];
			layoutBinding.stageFlags		= bindings.m_stages[i];
			layoutBindings.push_back(layoutBinding);
		}

		/*
			typedef struct VkDescriptorSetLayoutCreateInfo {
				VkStructureType                        sType;
				const void*                            pNext;
				VkDescriptorSetLayoutCreateFlags       flags;
				uint32_t                               bindingCount;
				const VkDescriptorSetLayoutBinding*    pBindings;
			} VkDescriptorSetLayoutCreateInfo;
		*/
		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.flags			= vk::DescriptorSetLayoutCreateFlagBits();
		layoutInfo.bindingCount		= bindings.m_count;
		layoutInfo.pBindings		= layoutBindings.data();

		try 
		{
			return device.createDescriptorSetLayout(layoutInfo);
		}
		catch (vk::SystemError err) 
		{
			std::cout << "Failed to create Descriptor Set Layout" << std::endl;

			return nullptr;
		}
	}

	vk::DescriptorPool CreateDescriptorPool(vk::Device device, uint32_t size, const DescriptorSetLayoutData& bindings)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;

		for (int i = 0; i < bindings.m_count; i++)
		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = bindings.m_types[i];
			poolSize.descriptorCount = size;
			poolSizes.push_back(poolSize);
		}

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.flags = vk::DescriptorPoolCreateFlags();
		poolInfo.maxSets = size;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		try 
		{
			return device.createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError err)
		{
			throw std::runtime_error("Failed to make descriptor pool");
		}
	}

	vk::DescriptorSet AllocateDescriptorSet(vk::Device device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout layout)
	{
		vk::DescriptorSetAllocateInfo allocationInfo;

		allocationInfo.descriptorPool = descriptorPool;
		allocationInfo.descriptorSetCount = 1;
		allocationInfo.pSetLayouts = &layout;

		try
		{
			return device.allocateDescriptorSets(allocationInfo)[0];
		}
		catch (vk::SystemError err)
		{
			throw std::runtime_error("Failed to allocate descriptor set from pool");
		}
	}
}