#pragma once
#include "Config.h"

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
	class SwapChainFrame 
	{
	public:
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;

		// Swapchain
		vk::Image image;
		vk::ImageView imageView;
		vk::Framebuffer framebuffer;
		vk::Image depthBuffer;
		vk::DeviceMemory depthBufferMemory;
		vk::ImageView depthBufferView;
		vk::Format depthFormat;
		int width, height;

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

		void CreateDescriptorResources();

		void CreateDepthResources();

		void WriteDescriptorSet();

		void Destroy();
	};

}