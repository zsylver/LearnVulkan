#include "SingleTimeCommands.h"

void vkUtil::StartJob(vk::CommandBuffer commandBuffer) 
{
	commandBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);
}

void vkUtil::EndJob(vk::CommandBuffer commandBuffer, vk::Queue submissionQueue) 
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	(void) submissionQueue.submit(1, &submitInfo, nullptr);
	submissionQueue.waitIdle();
}