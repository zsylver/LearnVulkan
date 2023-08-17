#pragma once
#include "Config.h"
#include "Memory.h"

namespace vkUtil 
{
	struct UBO
	{
		glm::mat4 m_view;
		glm::mat4 m_projection;
		glm::mat4 m_viewProjection;
	};

	/**
		Holds the data structures associated with a "Frame"
	*/
	struct SwapChainFrame 
	{
		// Swapchain
		vk::Image image;
		vk::ImageView imageView;
		vk::Framebuffer framebuffer;

		vk::CommandBuffer commandBuffer;

		// Synchronization
		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlight;

		// Resources
		UBO cameraData;
		Buffer cameraDataBuffer;
		void* cameraDataWriteLocation;
		std::vector<glm::mat4> modelTransforms;
		Buffer modelBuffer;
		void* modelBufferWriteLocation;

		// Resource descriptors
		vk::DescriptorBufferInfo uniformBufferDescriptor;
		vk::DescriptorBufferInfo modelBufferDescriptor;
		vk::DescriptorSet descriptorSet;	

		void CreateDescriptorResources(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice)
		{
			BufferInputChunk input;
			input.m_logicalDevice = logicalDevice;
			input.m_physicalDevice = physicalDevice;
			input.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.m_size = sizeof(UBO);
			input.m_usage = vk::BufferUsageFlagBits::eUniformBuffer;
			cameraDataBuffer = CreateBuffer(input);

			cameraDataWriteLocation = logicalDevice.mapMemory(cameraDataBuffer.m_bufferMemory, 0, sizeof(UBO));

			input.m_size = 1024 * sizeof(glm::mat4);
			input.m_usage = vk::BufferUsageFlagBits::eStorageBuffer;
			modelBuffer = CreateBuffer(input);

			modelBufferWriteLocation = logicalDevice.mapMemory(modelBuffer.m_bufferMemory, 0, 1024 * sizeof(glm::mat4));

			modelTransforms.reserve(1024);
			for (int i = 0; i < 1024; ++i)
				modelTransforms.push_back(glm::mat4(1.f));

			uniformBufferDescriptor.buffer = cameraDataBuffer.m_buffer;
			uniformBufferDescriptor.offset = 0;
			uniformBufferDescriptor.range = sizeof(UBO);

			modelBufferDescriptor.buffer = modelBuffer.m_buffer;
			modelBufferDescriptor.offset = 0;
			modelBufferDescriptor.range = 1024 * sizeof(glm::mat4);
		}

		void WriteDescriptorSet(vk::Device device)
		{
			vk::WriteDescriptorSet writeInfo;

			writeInfo.dstSet = descriptorSet;
			writeInfo.dstBinding = 0;
			writeInfo.dstArrayElement = 0;
			writeInfo.descriptorCount = 1;
			writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
			writeInfo.pBufferInfo = &uniformBufferDescriptor;

			device.updateDescriptorSets(writeInfo, nullptr);

			vk::WriteDescriptorSet writeInfo2;
			writeInfo2.dstSet = descriptorSet;
			writeInfo2.dstBinding = 1;
			writeInfo2.dstArrayElement = 0;
			writeInfo2.descriptorCount = 1;
			writeInfo2.descriptorType = vk::DescriptorType::eStorageBuffer;
			writeInfo2.pBufferInfo = &modelBufferDescriptor;

			device.updateDescriptorSets(writeInfo2, nullptr);
		}
	};

}