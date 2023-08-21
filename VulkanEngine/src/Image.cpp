#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Memory.h"
#include "Logging.h"
#include "SingleTimeCommands.h"
#include "Descriptors.h"

vk::Image vkImage::CreateImage(ImageInputChunk input)
{
	/*
	typedef struct VkImageCreateInfo {
		VkStructureType          sType;
		const void* pNext;
		VkImageCreateFlags       flags;
		VkImageType              imageType;
		VkFormat                 format;
		VkExtent3D               extent;
		uint32_t                 mipLevels;
		uint32_t                 arrayLayers;
		VkSampleCountFlagBits    samples;
		VkImageTiling            tiling;
		VkImageUsageFlags        usage;
		VkSharingMode            sharingMode;
		uint32_t                 queueFamilyIndexCount;
		const uint32_t* pQueueFamilyIndices;
		VkImageLayout            initialLayout;
	} VkImageCreateInfo;
	*/

	vk::ImageCreateInfo imageInfo;
	imageInfo.flags = vk::ImageCreateFlagBits() | input.m_flags;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(input.m_width, input.m_height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = input.m_arrayCount;
	imageInfo.format = input.m_format;
	imageInfo.tiling = input.m_tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = input.m_usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	try 
	{
		return input.m_logicalDevice.createImage(imageInfo);
	}
	catch (vk::SystemError err) 
	{
		throw std::runtime_error("Unable to make image");
	}
}

vk::DeviceMemory vkImage::CreateImageMemory(ImageInputChunk input, vk::Image image) 
{
	vk::MemoryRequirements requirements = input.m_logicalDevice.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocation;
	allocation.allocationSize = requirements.size;
	allocation.memoryTypeIndex = vkUtil::FindMemoryTypeIndex(input.m_physicalDevice, requirements.memoryTypeBits, input.m_memoryProperties);

	try
	{
		vk::DeviceMemory imageMemory = input.m_logicalDevice.allocateMemory(allocation);
		input.m_logicalDevice.bindImageMemory(image, imageMemory, 0);
		return imageMemory;
	}
	catch (vk::SystemError err) 
	{
		throw std::runtime_error("Unable to allocate memory for image");
	}
}

void vkImage::TransitionImageLayout(ImageLayoutTransitionJob transitionJob) 
{

	vkUtil::StartJob(transitionJob.m_commandBuffer);

	/*
	typedef struct VkImageSubresourceRange {
		VkImageAspectFlags    aspectMask;
		uint32_t              baseMipLevel;
		uint32_t              levelCount;
		uint32_t              baseArrayLayer;
		uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = transitionJob.m_arrayCount;

	/*
	typedef struct VkImageMemoryBarrier {
		VkStructureType            sType;
		const void* pNext;
		VkAccessFlags              srcAccessMask;
		VkAccessFlags              dstAccessMask;
		VkImageLayout              oldLayout;
		VkImageLayout              newLayout;
		uint32_t                   srcQueueFamilyIndex;
		uint32_t                   dstQueueFamilyIndex;
		VkImage                    image;
		VkImageSubresourceRange    subresourceRange;
	} VkImageMemoryBarrier;
	*/
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = transitionJob.m_oldLayout;
	barrier.newLayout = transitionJob.m_newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = transitionJob.m_image;
	barrier.subresourceRange = access;

	vk::PipelineStageFlags sourceStage, destinationStage;

	if (transitionJob.m_oldLayout == vk::ImageLayout::eUndefined
		&& transitionJob.m_newLayout == vk::ImageLayout::eTransferDstOptimal) 
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else 
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}

	transitionJob.m_commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

	vkUtil::EndJob(transitionJob.m_commandBuffer, transitionJob.m_queue);
}

void vkImage::CopyBufferToImage(BufferImageCopyJob copyJob) {

	vkUtil::StartJob(copyJob.m_commandBuffer);

	/*
	typedef struct VkBufferImageCopy {
		VkDeviceSize                bufferOffset;
		uint32_t                    bufferRowLength;
		uint32_t                    bufferImageHeight;
		VkImageSubresourceLayers    imageSubresource;
		VkOffset3D                  imageOffset;
		VkExtent3D                  imageExtent;
	} VkBufferImageCopy;
	*/
	vk::BufferImageCopy copy;
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;

	vk::ImageSubresourceLayers access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.mipLevel = 0;
	access.baseArrayLayer = 0;
	access.layerCount = copyJob.m_arrayCount;
	copy.imageSubresource = access;

	copy.imageOffset = vk::Offset3D(0, 0, 0);
	copy.imageExtent = vk::Extent3D(copyJob.m_width, copyJob.m_height, 1);

	copyJob.m_commandBuffer.copyBufferToImage(copyJob.m_srcBuffer, copyJob.m_dstImage, vk::ImageLayout::eTransferDstOptimal, copy);

	vkUtil::EndJob(copyJob.m_commandBuffer, copyJob.m_queue);
}

vk::ImageView vkImage::CreateImageView(vk::Device logicalDevice, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType type, uint32_t array_count)
{
	/*
	* ImageViewCreateInfo( VULKAN_HPP_NAMESPACE::ImageViewCreateFlags flags_ = {},
		VULKAN_HPP_NAMESPACE::Image                image_ = {},
		VULKAN_HPP_NAMESPACE::ImageViewType    viewType_  = VULKAN_HPP_NAMESPACE::ImageViewType::e1D,
		VULKAN_HPP_NAMESPACE::Format           format_    = VULKAN_HPP_NAMESPACE::Format::eUndefined,
		VULKAN_HPP_NAMESPACE::ComponentMapping components_            = {},
		VULKAN_HPP_NAMESPACE::ImageSubresourceRange subresourceRange_ = {} ) VULKAN_HPP_NOEXCEPT
	*/

	vk::ImageViewCreateInfo createInfo = {};
	createInfo.image = image;
	createInfo.viewType = type;
	createInfo.format = format;
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = array_count;

	return logicalDevice.createImageView(createInfo);
}

vk::Format vkImage::FindSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties properties = physicalDevice.getFormatProperties(format);

		/*
		typedef struct VkFormatProperties 
		{
			VkFormatFeatureFlags    linearTilingFeatures;
			VkFormatFeatureFlags    optimalTilingFeatures;
			VkFormatFeatureFlags    bufferFeatures;
		} VkFormatProperties;
		*/

		if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) 
		{
			return format;
		}

		if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) 
		{
			return format;
		}

		std::runtime_error("Unable to find suitable format");
	}

	return vk::Format();
}
