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

		// Resource descriptors
		vk::DescriptorBufferInfo uniformBufferDescriptor;
		vk::DescriptorSet descriptorSet;	

		void CreateUBOResources(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice)
		{
			BufferInputChunk input;
			input.m_logicalDevice = logicalDevice;
			input.m_physicalDevice = physicalDevice;
			input.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.m_size = sizeof(UBO);
			input.m_usage = vk::BufferUsageFlagBits::eUniformBuffer;
			cameraDataBuffer = CreateBuffer(input);

			cameraDataWriteLocation = logicalDevice.mapMemory(cameraDataBuffer.m_bufferMemory, 0, sizeof(UBO));

			uniformBufferDescriptor.buffer = cameraDataBuffer.m_buffer;
			uniformBufferDescriptor.offset = 0;
			uniformBufferDescriptor.range = sizeof(UBO);
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
		}
	};

}