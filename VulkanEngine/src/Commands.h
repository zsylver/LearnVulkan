#pragma once
#include "Config.h"
#include "QueueFamilies.h"
#include "Frame.h"

namespace vkInit 
{

	/**
		Data structures used in creating command buffers
	*/
	struct CommandBufferInputChunk 
	{
		vk::Device device;
		vk::CommandPool commandPool;
		std::vector<vkUtil::SwapChainFrame>& frames;
	};

	/**
		Make a command pool.
		\param device the logical device
		\param physicalDevice the physical device
		\param the windows surface (used for getting the queue families)
		\param debug whether the system is running in debug mode
		\returns the created command pool
	*/
	vk::CommandPool CreateCommandPool(
		vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool debug) 
	{

		vkUtil::QueueFamilyIndices queueFamilyIndices = vkUtil::FindQueueFamilies(physicalDevice, surface, debug);

		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		try 
		{
			return device.createCommandPool(poolInfo);
		}
		catch (vk::SystemError err) 
		{
			throw std::runtime_error("Error: Failed to create Command Pool!\n");
		}
	}

	/**
		Make a command buffer for each swapchain frame and return a main command buffer.
		\param inputChunk the required input info
		\param debug whether the system is running in debug mode
		\returns the main command buffer
	*/
	vk::CommandBuffer CreateCommandBuffers(CommandBufferInputChunk inputChunk, bool debug) {

		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = inputChunk.commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		//Make a command buffer for each frame
		for (int i = 0; i < inputChunk.frames.size(); ++i) 
		{
			try 
			{
				inputChunk.frames[i].commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];

				if (debug) 
				{
					std::cout << "Allocated command buffer for frame " << i << std::endl;
				}
			}
			catch (vk::SystemError err) 
			{
				if (debug) 
				{
					std::cout << "Failed to allocate command buffer for frame " << i << std::endl;
				}
			}
		}


		//Make a "main" command buffer for the engine
		try 
		{
			vk::CommandBuffer commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];

			if (debug) 
			{
				std::cout << "Allocated main command buffer " << std::endl;
			}

			return commandBuffer;
		}
		catch (vk::SystemError err) 
		{

			if (debug) 
			{
				std::cout << "Failed to allocate main command buffer " << std::endl;
			}

			return nullptr;
		}
	}
}