#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Memory.h"
#include "Logging.h"
#include "SingleTimeCommands.h"
#include "Descriptors.h"

vkImage::Texture::Texture(TextureInputChunk input) : 
	m_logicalDevice{ input.m_logicalDevice },
	m_physicalDevice{ input.m_physicalDevice },
	m_filename{ input.m_filename },
	m_commandBuffer{ input.m_commandBuffer },
	m_queue{ input.m_queue },
	m_layout{ input.m_layout },
	m_descriptorPool{ input.m_descriptorPool }
{
	Load();

	ImageInputChunk imageInput;
	imageInput.m_logicalDevice = m_logicalDevice;
	imageInput.m_physicalDevice = m_physicalDevice;
	imageInput.m_width = m_width;
	imageInput.m_height = m_height;
	imageInput.m_tiling = vk::ImageTiling::eOptimal;
	imageInput.m_usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	imageInput.m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

	m_image = CreateImage(imageInput);
	m_imageMemory = CreateImageMemory(imageInput, m_image);

	Populate();

	free(m_pixels);

	CreateView();

	CreateSampler();

	CreateDescriptorSet();
}

vkImage::Texture::~Texture()
{
	m_logicalDevice.freeMemory(m_imageMemory);
	m_logicalDevice.destroyImage(m_image);
	m_logicalDevice.destroyImageView(m_imageView);
	m_logicalDevice.destroySampler(m_sampler);
}

void vkImage::Texture::Load()
{
	m_pixels = stbi_load(m_filename, &m_width, &m_height, &m_channels, STBI_rgb_alpha);
	if (!m_pixels)
		std::cout << "Failed to load: " << m_filename << std::endl;
}

void vkImage::Texture::Populate()
{
	//First create a CPU-visible buffer...
	BufferInputChunk input;
	input.m_logicalDevice = m_logicalDevice;
	input.m_physicalDevice = m_physicalDevice;
	input.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	input.m_usage = vk::BufferUsageFlagBits::eTransferSrc;
	input.m_size = m_width * m_height * 4; // 4 bytes

	Buffer stagingBuffer = vkUtil::CreateBuffer(input);

	//...then fill it,
	void* writeLocation = m_logicalDevice.mapMemory(stagingBuffer.m_bufferMemory, 0, input.m_size);
	memcpy(writeLocation, m_pixels, input.m_size);
	m_logicalDevice.unmapMemory(stagingBuffer.m_bufferMemory);

	//then transfer it to image memory
	ImageLayoutTransitionJob transitionJob;
	transitionJob.m_commandBuffer = m_commandBuffer;
	transitionJob.m_queue = m_queue;
	transitionJob.m_image = m_image;
	transitionJob.m_oldLayout = vk::ImageLayout::eUndefined;
	transitionJob.m_newLayout = vk::ImageLayout::eTransferDstOptimal;
	TransitionImageLayout(transitionJob);

	BufferImageCopyJob copyJob;
	copyJob.m_commandBuffer = m_commandBuffer;
	copyJob.m_queue = m_queue;
	copyJob.m_srcBuffer = stagingBuffer.m_buffer;
	copyJob.m_dstImage = m_image;
	copyJob.m_width = m_width;
	copyJob.m_height = m_height;
	CopyBufferToImage(copyJob);

	transitionJob.m_oldLayout = vk::ImageLayout::eTransferDstOptimal;
	transitionJob.m_newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	TransitionImageLayout(transitionJob);

	//Now the staging buffer can be destroyed
	m_logicalDevice.freeMemory(stagingBuffer.m_bufferMemory);
	m_logicalDevice.destroyBuffer(stagingBuffer.m_buffer);
}

void vkImage::Texture::CreateView() 
{
	m_imageView = CreateImageView(m_logicalDevice, m_image, vk::Format::eR8G8B8A8Unorm);
}

void vkImage::Texture::CreateSampler() {

	/*
	typedef struct VkSamplerCreateInfo {
		VkStructureType         sType;
		const void* pNext;
		VkSamplerCreateFlags    flags;
		VkFilter                magFilter;
		VkFilter                minFilter;
		VkSamplerMipmapMode     mipmapMode;
		VkSamplerAddressMode    addressModeU;
		VkSamplerAddressMode    addressModeV;
		VkSamplerAddressMode    addressModeW;
		float                   mipLodBias;
		VkBool32                anisotropyEnable;
		float                   maxAnisotropy;
		VkBool32                compareEnable;
		VkCompareOp             compareOp;
		float                   minLod;
		float                   maxLod;
		VkBorderColor           borderColor;
		VkBool32                unnormalizedCoordinates;
	} VkSamplerCreateInfo;
	*/
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.flags = vk::SamplerCreateFlags();
	samplerInfo.minFilter = vk::Filter::eNearest;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1.0f;

	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;

	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	try
	{
		m_sampler = m_logicalDevice.createSampler(samplerInfo);
	}
	catch (vk::SystemError err) 
	{
		throw std::runtime_error("Failed to make sampler.");
	}

}

void vkImage::Texture::CreateDescriptorSet() {

	m_descriptorSet = vkInit::AllocateDescriptorSet(m_logicalDevice, m_descriptorPool, m_layout);

	vk::DescriptorImageInfo imageDescriptor;
	imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageDescriptor.imageView = m_imageView;
	imageDescriptor.sampler = m_sampler;

	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = m_descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescriptor;

	m_logicalDevice.updateDescriptorSets(descriptorWrite, nullptr);
}

void vkImage::Texture::Use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout)
{
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, m_descriptorSet, nullptr);
}

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
	imageInfo.flags = vk::ImageCreateFlagBits();
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(input.m_width, input.m_height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageInfo.tiling = input.m_tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = input.m_usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	try {
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
	allocation.memoryTypeIndex = vkUtil::FindMemoryTypeIndex(input.m_physicalDevice, requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

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
	access.layerCount = 1;

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
	access.layerCount = 1;
	copy.imageSubresource = access;

	copy.imageOffset = vk::Offset3D(0, 0, 0);
	copy.imageExtent = vk::Extent3D(copyJob.m_width, copyJob.m_height, 1.0f);

	copyJob.m_commandBuffer.copyBufferToImage(copyJob.m_srcBuffer, copyJob.m_dstImage, vk::ImageLayout::eTransferDstOptimal, copy);

	vkUtil::EndJob(copyJob.m_commandBuffer, copyJob.m_queue);
}

vk::ImageView vkImage::CreateImageView(vk::Device logicalDevice, vk::Image image, vk::Format format) 
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
	createInfo.viewType = vk::ImageViewType::e2D;
	createInfo.format = format;
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;
	createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	return logicalDevice.createImageView(createInfo);
}