#include "Frame.h"
#include "Image.h"
#include "Memory.h"

void vkUtil::SwapChainFrame::CreateDescriptorResources()
{
	BufferInputChunk input;
	input.m_logicalDevice = logicalDevice;
	input.m_physicalDevice = physicalDevice;
	input.m_memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	input.m_size = sizeof(CameraVectors);
	input.m_usage = vk::BufferUsageFlagBits::eUniformBuffer;
	cameraVectorBuffer = CreateBuffer(input);

	cameraVectorWriteLocation = logicalDevice.mapMemory(cameraVectorBuffer.m_bufferMemory, 0, sizeof(CameraVectors));

	input.m_size = sizeof(CameraMatrices);
	cameraMatrixBuffer = CreateBuffer(input);

	cameraMatrixWriteLocation = logicalDevice.mapMemory(cameraMatrixBuffer.m_bufferMemory, 0, sizeof(CameraMatrices));

	input.m_size = 1024 * sizeof(glm::mat4);
	input.m_usage = vk::BufferUsageFlagBits::eStorageBuffer;
	modelBuffer = CreateBuffer(input);

	modelBufferWriteLocation = logicalDevice.mapMemory(modelBuffer.m_bufferMemory, 0, 1024 * sizeof(glm::mat4));

	modelTransforms.reserve(1024);
	for (int i = 0; i < 1024; ++i)
		modelTransforms.push_back(glm::mat4(1.f));

	cameraVectorDescriptor.buffer = cameraVectorBuffer.m_buffer;
	cameraVectorDescriptor.offset = 0;
	cameraVectorDescriptor.range = sizeof(CameraVectors);

	cameraMatrixDescriptor.buffer = cameraMatrixBuffer.m_buffer;
	cameraMatrixDescriptor.offset = 0;
	cameraMatrixDescriptor.range = sizeof(CameraMatrices);

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
	imageInfo.m_arrayCount = 1;

	depthBuffer = vkImage::CreateImage(imageInfo);
	depthBufferMemory = vkImage::CreateImageMemory(imageInfo, depthBuffer);
	depthBufferView = vkImage::CreateImageView(logicalDevice, depthBuffer, depthFormat, vk::ImageAspectFlagBits::eDepth, vk::ImageViewType::e2D, 1);
}

void vkUtil::SwapChainFrame::WriteDescriptorSet()
{
	logicalDevice.updateDescriptorSets(writeOps, nullptr);
}

void vkUtil::SwapChainFrame::RecordWriteOperations()
{
	vk::WriteDescriptorSet cameraVectorWrite;

	cameraVectorWrite.dstSet = descriptorSet[PipelineTypes::SKY];
	cameraVectorWrite.dstBinding = 0;
	cameraVectorWrite.dstArrayElement = 0;
	cameraVectorWrite.descriptorCount = 1;
	cameraVectorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraVectorWrite.pBufferInfo = &cameraVectorDescriptor;

	vk::WriteDescriptorSet cameraMatrixWrite;

	cameraMatrixWrite.dstSet = descriptorSet[PipelineTypes::STANDARD];
	cameraMatrixWrite.dstBinding = 0;
	cameraMatrixWrite.dstArrayElement = 0;
	cameraMatrixWrite.descriptorCount = 1;
	cameraMatrixWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraMatrixWrite.pBufferInfo = &cameraMatrixDescriptor;

	vk::WriteDescriptorSet ssboWrite;
	ssboWrite.dstSet = descriptorSet[PipelineTypes::STANDARD];
	ssboWrite.dstBinding = 1;
	ssboWrite.dstArrayElement = 0;
	ssboWrite.descriptorCount = 1;
	ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
	ssboWrite.pBufferInfo = &modelBufferDescriptor;

	writeOps = { { cameraVectorWrite, cameraMatrixWrite, ssboWrite } };
}

void vkUtil::SwapChainFrame::Destroy()
{
	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroyFramebuffer(framebuffer[PipelineTypes::SKY]);
	logicalDevice.destroyFramebuffer(framebuffer[PipelineTypes::STANDARD]);
	logicalDevice.destroyFence(inFlight);
	logicalDevice.destroySemaphore(imageAvailable);
	logicalDevice.destroySemaphore(renderFinished);

	logicalDevice.unmapMemory(cameraVectorBuffer.m_bufferMemory);
	logicalDevice.freeMemory(cameraVectorBuffer.m_bufferMemory);
	logicalDevice.destroyBuffer(cameraVectorBuffer.m_buffer);

	logicalDevice.unmapMemory(cameraMatrixBuffer.m_bufferMemory);
	logicalDevice.freeMemory(cameraMatrixBuffer.m_bufferMemory);
	logicalDevice.destroyBuffer(cameraMatrixBuffer.m_buffer);

	logicalDevice.unmapMemory(modelBuffer.m_bufferMemory);
	logicalDevice.freeMemory(modelBuffer.m_bufferMemory);
	logicalDevice.destroyBuffer(modelBuffer.m_buffer);

	logicalDevice.destroyImage(depthBuffer);
	logicalDevice.freeMemory(depthBufferMemory);
	logicalDevice.destroyImageView(depthBufferView);
}
