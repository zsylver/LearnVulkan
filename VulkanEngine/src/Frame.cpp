#include "Frame.h"
#include "Image.h"
#include "Memory.h"

void vkUtil::SwapChainFrame::CreateDescriptorResources()
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

void vkUtil::SwapChainFrame::CreateDepthResources()
{
	depthFormat = vkImage::FindSupportedFormat(physicalDevice, { vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	vkImage::ImageInputChunk imageInfo;
	imageInfo.m_logicalDevice = logicalDevice;
	imageInfo.m_physicalDevice = physicalDevice;
	imageInfo.m_tiling = vk::ImageTiling::eOptimal;
	imageInfo.m_usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	imageInfo.m_width = width;
	imageInfo.m_height = height;
	imageInfo.m_format = depthFormat;

	depthBuffer = vkImage::CreateImage(imageInfo);
	depthBufferMemory = vkImage::CreateImageMemory(imageInfo, depthBuffer);
	depthBufferView = vkImage::CreateImageView(logicalDevice, depthBuffer, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void vkUtil::SwapChainFrame::WriteDescriptorSet()
{
	vk::WriteDescriptorSet writeInfo;

	writeInfo.dstSet = descriptorSet;
	writeInfo.dstBinding = 0;
	writeInfo.dstArrayElement = 0;
	writeInfo.descriptorCount = 1;
	writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
	writeInfo.pBufferInfo = &uniformBufferDescriptor;

	logicalDevice.updateDescriptorSets(writeInfo, nullptr);

	vk::WriteDescriptorSet writeInfo2;
	writeInfo2.dstSet = descriptorSet;
	writeInfo2.dstBinding = 1;
	writeInfo2.dstArrayElement = 0;
	writeInfo2.descriptorCount = 1;
	writeInfo2.descriptorType = vk::DescriptorType::eStorageBuffer;
	writeInfo2.pBufferInfo = &modelBufferDescriptor;

	logicalDevice.updateDescriptorSets(writeInfo2, nullptr);
}

void vkUtil::SwapChainFrame::Destroy()
{
	logicalDevice.destroyImage(depthBuffer);
	logicalDevice.freeMemory(depthBufferMemory);
	logicalDevice.destroyImageView(depthBufferView);

	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroyFramebuffer(framebuffer);
	logicalDevice.destroyFence(inFlight);
	logicalDevice.destroySemaphore(imageAvailable);
	logicalDevice.destroySemaphore(renderFinished);

	logicalDevice.unmapMemory(cameraDataBuffer.m_bufferMemory);
	logicalDevice.freeMemory(cameraDataBuffer.m_bufferMemory);
	logicalDevice.destroyBuffer(cameraDataBuffer.m_buffer);

	logicalDevice.unmapMemory(modelBuffer.m_bufferMemory);
	logicalDevice.freeMemory(modelBuffer.m_bufferMemory);
	logicalDevice.destroyBuffer(modelBuffer.m_buffer);
}
