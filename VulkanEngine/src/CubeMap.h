#pragma once
#include "Config.h"
#include "Image.h"

namespace vkImage
{

	class CubeMap
	{
	public:
		CubeMap(TextureInputChunk input);

		void Use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
		~CubeMap();
	private:
		int m_width;
		int m_height;
		int m_channels;
		vk::Device m_logicalDevice;
		vk::PhysicalDevice m_physicalDevice;
		std::vector<const char*> m_filenames;
		stbi_uc* m_pixels[6];

		//Resources
		vk::Image m_image;
		vk::DeviceMemory m_imageMemory;
		vk::ImageView m_imageView;
		vk::Sampler m_sampler;

		//Resource Descriptors
		vk::DescriptorSetLayout m_layout;
		vk::DescriptorSet m_descriptorSet;
		vk::DescriptorPool m_descriptorPool;

		vk::CommandBuffer m_commandBuffer;
		vk::Queue m_queue;

		void Load();

		void Populate();

		void CreateView();

		void CreateSampler();

		void CreateDescriptorSet();
	};
}