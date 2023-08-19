#pragma once
#include "Config.h"
#include "Logging.h"
#include "QueueFamilies.h"
#include "Frame.h"
#include "Image.h"

namespace vkInit {

	/**
		Holds properties of the swapchain
		capabilities: no. of images and supported sizes
		formats: eg. supported pixel formats
		present modes: available presentation modes (eg. double buffer, fifo, mailbox)
	*/
	struct SwapChainSupportDetails 
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	/**
		Various data structures associated with the swapchain.
	*/
	struct SwapChainBundle 
	{
		vk::SwapchainKHR swapchain;
		std::vector<vkUtil::SwapChainFrame> frames;
		vk::Format format;
		vk::Extent2D extent;
	};

	/**
		Check the supported swapchain parameters
		\param device the physical device
		\param surface the window surface which will use the swapchain
		\param debug whether the system is running in debug mode
		\returns a struct holding the details
	*/
	SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface, bool debug) 
	{
		SwapChainSupportDetails support;

		/*
		* typedef struct VkSurfaceCapabilitiesKHR {
			uint32_t                         minImageCount;
			uint32_t                         maxImageCount;
			VkExtent2D                       currentExtent;
			VkExtent2D                       minImageExtent;
			VkExtent2D                       maxImageExtent;
			uint32_t                         maxImageArrayLayers;
			VkSurfaceTransformFlagsKHR       supportedTransforms;
			VkSurfaceTransformFlagBitsKHR    currentTransform;
			VkCompositeAlphaFlagsKHR         supportedCompositeAlpha;
			VkImageUsageFlags                supportedUsageFlags;
		} VkSurfaceCapabilitiesKHR;
		*/
		support.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		if (debug) 
		{
			std::cout << "Swapchain can support the following surface capabilities:\n";

			std::cout << "\tminimum image count: " << support.capabilities.minImageCount << '\n';
			std::cout << "\tmaximum image count: " << support.capabilities.maxImageCount << '\n';

			std::cout << "\tcurrent extent: \n";
			/*typedef struct VkExtent2D {
				uint32_t    width;
				uint32_t    height;
			} VkExtent2D;
			*/
			std::cout << "\t\twidth: " << support.capabilities.currentExtent.width << '\n';
			std::cout << "\t\theight: " << support.capabilities.currentExtent.height << '\n';

			std::cout << "\tminimum supported extent: \n";
			std::cout << "\t\twidth: " << support.capabilities.minImageExtent.width << '\n';
			std::cout << "\t\theight: " << support.capabilities.minImageExtent.height << '\n';

			std::cout << "\tmaximum supported extent: \n";
			std::cout << "\t\twidth: " << support.capabilities.maxImageExtent.width << '\n';
			std::cout << "\t\theight: " << support.capabilities.maxImageExtent.height << '\n';

			std::cout << "\tmaximum image array layers: " << support.capabilities.maxImageArrayLayers << '\n';

			// Log all supported transforms
			std::cout << "\tsupported transforms:\n";
			std::vector<std::string> stringList = LogTransformBits(support.capabilities.supportedTransforms);
			for (std::string line : stringList) 
			{
				std::cout << "\t\t" << line << '\n';
			}
			
			// Log current transform
			std::cout << "\tcurrent transform:\n";
			stringList = LogTransformBits(support.capabilities.currentTransform);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << '\n';
			}

			// Log all supported alpha operations
			std::cout << "\tsupported alpha operations:\n";
			stringList = LogAlphaCompositeBits(support.capabilities.supportedCompositeAlpha);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << '\n';
			}

			// Log all supported image usage
			std::cout << "\tsupported image usage:\n";
			stringList = LogImageUsageBits(support.capabilities.supportedUsageFlags);
			for (std::string line : stringList) 
			{
				std::cout << "\t\t" << line << '\n';
			}
		}

		support.formats = device.getSurfaceFormatsKHR(surface);

		if (debug) 
		{
			for (vk::SurfaceFormatKHR supportedFormat : support.formats) 
			{
				/*
				* typedef struct VkSurfaceFormatKHR {
					VkFormat           format;
					VkColorSpaceKHR    colorSpace;
				} VkSurfaceFormatKHR;
				*/

				std::cout << "supported pixel format: " << vk::to_string(supportedFormat.format) << '\n';
				std::cout << "supported color space: " << vk::to_string(supportedFormat.colorSpace) << '\n';
			}
		}
		
		// Logging Present Mode information

		//support.presentModes = device.getSurfacePresentModesKHR(surface);

		//for (vk::PresentModeKHR presentMode : support.presentModes) 
		//{
		//	std::cout << '\t' << LogPresentMode(presentMode) << '\n';
		//}
		return support;
	}

	/**
		Choose a surface format for the swapchain
		\param formats a vector of surface formats supported by the device
		\returns the chosen format
	*/
	vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> formats)
	{
		for (vk::SurfaceFormatKHR format : formats) 
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm
				&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}

		return formats[0];
	}

	/**
		Choose a present mode.
		\param presentModes a vector of present modes supported by the device
		\returns the chosen present mode
	*/
	vk::PresentModeKHR ChooseSwapChainPresentMode(std::vector<vk::PresentModeKHR> presentModes) 
	{
		for (vk::PresentModeKHR presentMode : presentModes) 
		{
			if (presentMode == vk::PresentModeKHR::eMailbox) 
			{
				return presentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	/**
		Choose an extent for the swapchain.
		\param width the requested width
		\param height the requested height
		\param capabilities a struct describing the supported capabilities of the device
		\returns the chosen extent
	*/
	vk::Extent2D ChooseSwapChainExtent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR capabilities) 
	{
		if (capabilities.currentExtent.width != UINT32_MAX) 
		{
			return capabilities.currentExtent;
		}
		else 
		{
			vk::Extent2D extent = { width, height };

			extent.width = std::min(
				capabilities.maxImageExtent.width,
				std::max(capabilities.minImageExtent.width, extent.width)
			);

			extent.height = std::min(
				capabilities.maxImageExtent.height,
				std::max(capabilities.minImageExtent.height, extent.height)
			);

			return extent;
		}
	}

	/**
		Create a swapchain
		\param logicalDevice the logical device
		\param physicalDevice the physical device
		\param surface the window surface to use the swapchain with
		\param width the requested width
		\param height the requested height
		\param debug whether the system is running in debug mode
		\returns a struct holding the swapchain and other associated data structures
	*/
	SwapChainBundle CreateSwapChain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, int width, int height, bool debug) 
	{
		SwapChainSupportDetails support = QuerySwapChainSupport(physicalDevice, surface, debug);

		vk::SurfaceFormatKHR format = ChooseSwapChainSurfaceFormat(support.formats);

		vk::PresentModeKHR presentMode = ChooseSwapChainPresentMode(support.presentModes);

		vk::Extent2D extent = ChooseSwapChainExtent(width, height, support.capabilities);

		uint32_t imageCount = std::min(	support.capabilities.maxImageCount, support.capabilities.minImageCount + 1);

		/*
		* VULKAN_HPP_CONSTEXPR SwapchainCreateInfoKHR(
		  VULKAN_HPP_NAMESPACE::SwapchainCreateFlagsKHR flags_         = {},
		  VULKAN_HPP_NAMESPACE::SurfaceKHR              surface_       = {},
		  uint32_t                                      minImageCount_ = {},
		  VULKAN_HPP_NAMESPACE::Format                  imageFormat_   = VULKAN_HPP_NAMESPACE::Format::eUndefined,
		  VULKAN_HPP_NAMESPACE::ColorSpaceKHR   imageColorSpace_  = VULKAN_HPP_NAMESPACE::ColorSpaceKHR::eSrgbNonlinear,
		  VULKAN_HPP_NAMESPACE::Extent2D        imageExtent_      = {},
		  uint32_t                              imageArrayLayers_ = {},
		  VULKAN_HPP_NAMESPACE::ImageUsageFlags imageUsage_       = {},
		  VULKAN_HPP_NAMESPACE::SharingMode     imageSharingMode_ = VULKAN_HPP_NAMESPACE::SharingMode::eExclusive,
		  uint32_t                              queueFamilyIndexCount_ = {},
		  const uint32_t *                      pQueueFamilyIndices_   = {},
		  VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR preTransform_ =
			VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR::eIdentity,
		  VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR compositeAlpha_ =
			VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR::eOpaque,
		  VULKAN_HPP_NAMESPACE::PresentModeKHR presentMode_  = VULKAN_HPP_NAMESPACE::PresentModeKHR::eImmediate,
		  VULKAN_HPP_NAMESPACE::Bool32         clipped_      = {},
		  VULKAN_HPP_NAMESPACE::SwapchainKHR   oldSwapchain_ = {} ) VULKAN_HPP_NOEXCEPT
		*/
		vk::SwapchainCreateInfoKHR swapChainCreateInfo
		{
			vk::SwapchainCreateFlagsKHR(),
			surface,
			imageCount,
			format.format,
			format.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment
		};

		vkUtil::QueueFamilyIndices indices{ vkUtil::FindQueueFamilies(physicalDevice, surface, debug) };
		uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) 
		{
			// 2 concurrent queues
			swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else 
		{
			// 1 queue
			swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		swapChainCreateInfo.preTransform = support.capabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;

		swapChainCreateInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		SwapChainBundle bundle{};
		try 
		{
			bundle.swapchain = logicalDevice.createSwapchainKHR(swapChainCreateInfo);
		}
		catch (vk::SystemError err) 
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		std::vector<vk::Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
		bundle.frames.resize(images.size());

		for (size_t i = 0; i < images.size(); ++i) 
		{

			/*
			* ImageViewCreateInfo( VULKAN_HPP_NAMESPACE::ImageViewCreateFlags flags_ = {},
						   VULKAN_HPP_NAMESPACE::Image                image_ = {},
						   VULKAN_HPP_NAMESPACE::ImageViewType    viewType_  = VULKAN_HPP_NAMESPACE::ImageViewType::e1D,
						   VULKAN_HPP_NAMESPACE::Format           format_    = VULKAN_HPP_NAMESPACE::Format::eUndefined,
						   VULKAN_HPP_NAMESPACE::ComponentMapping components_            = {},
						   VULKAN_HPP_NAMESPACE::ImageSubresourceRange subresourceRange_ = {} ) VULKAN_HPP_NOEXCEPT
				: flags( flags_ )
				, image( image_ )
				, viewType( viewType_ )
				, format( format_ )
				, components( components_ )
				, subresourceRange( subresourceRange_ )
			*/

			vk::ImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.image = images[i];
			imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
			imageViewCreateInfo.format = format.format;
			imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			bundle.frames[i].image = images[i];
			bundle.frames[i].imageView = vkImage::CreateImageView(logicalDevice, images[i], format.format);
		}

		bundle.format = format.format;
		bundle.extent = extent;

		return bundle;
	}
}