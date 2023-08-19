#pragma once
#include "Config.h"
namespace vkUtil 
{
	/**
		Begin recording a command buffer intended for a single submit.
	*/
	void StartJob(vk::CommandBuffer commandBuffer);

	/**
		Finish recording a command buffer and submit it.
	*/
	void EndJob(vk::CommandBuffer commandBuffer, vk::Queue submissionQueue);
}