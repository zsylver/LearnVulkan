#pragma once
#include "config.h"
#include "frame.h"

namespace vkInit 
{
	/**
		Data structures involved in making framebuffers for the
		swapchain.
	*/
	struct FramebufferInput 
	{
		vk::Device device;
		std::unordered_map<PipelineTypes, vk::RenderPass> renderPass;
		vk::Extent2D swapChainExtent;
	};

	/**
		Make framebuffers for the swapchain
		\param inputChunk required input for creation
		\param frames the vector to be populated with the created framebuffers
		\param debug whether the system is running in debug mode.
	*/
	void CreateFramebuffers(FramebufferInput inputChunk, std::vector<vkUtil::SwapChainFrame>& frames, bool debug) 
	{
		for (int i = 0; i < frames.size(); ++i) 
		{
			std::vector<vk::ImageView> attachments
			{
				frames[i].imageView
			};

			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = inputChunk.renderPass[PipelineTypes::SKY];
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = inputChunk.swapChainExtent.width;
			framebufferInfo.height = inputChunk.swapChainExtent.height;
			framebufferInfo.layers = 1;

			try
			{
				frames[i].framebuffer[PipelineTypes::SKY] = inputChunk.device.createFramebuffer(framebufferInfo);

				if (debug)
					std::cout << "Created Sky framebuffer for frame " << i << std::endl;
			}
			catch (vk::SystemError err)
			{
				if (debug)
					std::cout << "Failed to create Sky framebuffer for frame " << i << std::endl;
			}


			attachments.push_back(frames[i].depthBufferView);

			framebufferInfo.renderPass = inputChunk.renderPass[PipelineTypes::STANDARD];
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();

			try 
			{
				frames[i].framebuffer[PipelineTypes::STANDARD] = inputChunk.device.createFramebuffer(framebufferInfo);

				if (debug)
					std::cout << "Created standard framebuffer for frame " << i << std::endl;
			}
			catch (vk::SystemError err) 
			{
				if (debug)
					std::cout << "Failed to create standard framebuffer for frame " << i << std::endl;
			}

		}
	}

}