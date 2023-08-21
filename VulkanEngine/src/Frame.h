#pragma once
#include "Config.h"

namespace vkUtil 
{
	struct CameraMatrices
	{
		glm::mat4 m_view;
		glm::mat4 m_projection;
		glm::mat4 m_viewProjection;
	};

	struct CameraVectors
	{
		glm::vec4 m_forward;
		glm::vec4 m_right;
		glm::vec4 m_up;
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
		std::unordered_map<PipelineTypes, vk::Framebuffer> framebuffer;
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
		CameraMatrices cameraMatrixData;
		Buffer cameraMatrixBuffer;
		void* cameraMatrixWriteLocation;

		CameraVectors cameraVectorData;
		Buffer cameraVectorBuffer;
		void* cameraVectorWriteLocation;

		std::vector<glm::mat4> modelTransforms;
		Buffer modelBuffer;
		void* modelBufferWriteLocation;

		// Resource descriptors
		vk::DescriptorBufferInfo cameraVectorDescriptor;
		vk::DescriptorBufferInfo cameraMatrixDescriptor;
		vk::DescriptorBufferInfo modelBufferDescriptor;
		std::unordered_map<PipelineTypes, vk::DescriptorSet> descriptorSet;

		//Write Ops
		std::vector<vk::WriteDescriptorSet> writeOps;

		void CreateDescriptorResources();

		void CreateDepthResources();

		void WriteDescriptorSet();

		void RecordWriteOperations();

		void Destroy();
	};

}