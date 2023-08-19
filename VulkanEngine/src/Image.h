#pragma once
#include "stb_image.h"
#include "Config.h"

namespace vkImage
{
	struct TextureInputChunk
	{
		vk::Device m_logicalDevice;
		vk::PhysicalDevice m_physicalDevice;
		const char* m_filename;
		vk::CommandBuffer m_commandBuffer;
		vk::Queue m_queue;
		vk::DescriptorSetLayout m_layout;
		vk::DescriptorPool m_descriptorPool;
	};

	struct ImageInputChunk
	{
		vk::Device m_logicalDevice;
		vk::PhysicalDevice m_physicalDevice;
		int m_width, m_height;
		vk::ImageTiling m_tiling;
		vk::ImageUsageFlags m_usage;
		vk::MemoryPropertyFlags m_memoryProperties;
	};

	struct ImageLayoutTransitionJob
	{
		vk::CommandBuffer m_commandBuffer;
		vk::Queue m_queue;
		vk::Image m_image;
		vk::ImageLayout m_oldLayout, m_newLayout;
	};

	struct BufferImageCopyJob
	{
		vk::CommandBuffer m_commandBuffer;
		vk::Queue m_queue;
		vk::Buffer m_srcBuffer;
		vk::Image m_dstImage;
		int m_width, m_height;
	};

	class Texture
	{
	public:
		Texture(TextureInputChunk input);

		void Use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
		~Texture();
	private:
		int m_width;
		int m_height;
		int m_channels;
		vk::Device m_logicalDevice;
		vk::PhysicalDevice m_physicalDevice;
		const char* m_filename;
		stbi_uc* m_pixels;

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

	vk::Image CreateImage(ImageInputChunk input);

	vk::DeviceMemory CreateImageMemory(ImageInputChunk input, vk::Image image);

	void TransitionImageLayout(ImageLayoutTransitionJob job);

	void CopyBufferToImage(BufferImageCopyJob job);

	vk::ImageView CreateImageView(vk::Device logicalDevice, vk::Image image, vk::Format format);
}