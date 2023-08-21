#include "Texture.h"
#include "stb_image.h"
#include "Memory.h"
#include "Logging.h"
#include "Descriptors.h"

vkImage::Texture::Texture(TextureInputChunk input) :
	m_logicalDevice{ input.m_logicalDevice },
	m_physicalDevice{ input.m_physicalDevice },
	m_filename{ input.m_filenames[0]},
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
	imageInput.m_arrayCount = 1;
	imageInput.m_format = vk::Format::eR8G8B8A8Unorm;
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
	transitionJob.m_arrayCount = 1;
	TransitionImageLayout(transitionJob);

	BufferImageCopyJob copyJob;
	copyJob.m_commandBuffer = m_commandBuffer;
	copyJob.m_queue = m_queue;
	copyJob.m_srcBuffer = stagingBuffer.m_buffer;
	copyJob.m_dstImage = m_image;
	copyJob.m_width = m_width;
	copyJob.m_height = m_height;
	copyJob.m_arrayCount = 1;
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
	m_imageView = CreateImageView(m_logicalDevice, m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D, 1);
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