#pragma once
#include "Config.h"

namespace vkInit 
{
	/**
		Create semaphore.
		\param device the logical device
		\param debug whether the system is running in debug mode
		\returns the created semaphore
	*/
	vk::Semaphore CreateSemaphore(vk::Device device, bool debug) 
	{
		vk::SemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.flags = vk::SemaphoreCreateFlags();

		try 
		{
			return device.createSemaphore(semaphoreInfo);
		}
		catch (vk::SystemError err) 
		{
			if (debug) 
				std::cout << "Failed to create semaphore " << std::endl;

			return nullptr;
		}
	}

	/**
		Create fence.
		\param device the logical device
		\param debug whether the system is running in debug mode
		\returns the created fence
	*/
	vk::Fence CreateFence(vk::Device device, bool debug) 
	{
		vk::FenceCreateInfo fenceInfo{};
		fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		try 
		{
			return device.createFence(fenceInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug)
				std::cout << "Failed to create fence " << std::endl;
			
			return nullptr;
		}
	}
}