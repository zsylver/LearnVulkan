#pragma once
#include "stb_image.h"
#include "Config.h"

namespace vkImage
{
	struct TextureInputChunk
	{
		vk::Device m_logicalDevice;
		vk::PhysicalDevice m_physicalDevice;
		std::vector<const char*> m_filenames;
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
		vk::Format m_format;
		uint32_t m_arrayCount;
		vk::ImageCreateFlags m_flags;
	};

	struct ImageLayoutTransitionJob
	{
		vk::CommandBuffer m_commandBuffer;
		vk::Queue m_queue;
		vk::Image m_image;
		vk::ImageLayout m_oldLayout, m_newLayout;
		uint32_t m_arrayCount;
	};

	struct BufferImageCopyJob
	{
		vk::CommandBuffer m_commandBuffer;
		vk::Queue m_queue;
		vk::Buffer m_srcBuffer;
		vk::Image m_dstImage;
		int m_width, m_height;
		uint32_t m_arrayCount;
	};

	vk::Image CreateImage(ImageInputChunk input);

	vk::DeviceMemory CreateImageMemory(ImageInputChunk input, vk::Image image);

	void TransitionImageLayout(ImageLayoutTransitionJob job);

	void CopyBufferToImage(BufferImageCopyJob job);

	vk::ImageView CreateImageView(vk::Device logicalDevice, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType type, uint32_t array_count);

	vk::Format FindSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
}