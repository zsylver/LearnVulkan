#pragma once
#include "Config.h"

namespace vkInit 
{
	/*
	* Debug call back:
	*
	*	typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
		} VkDebugUtilsMessageSeverityFlagBitsEXT;
	*	typedef enum VkDebugUtilsMessageTypeFlagBitsEXT {
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,
			VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
		} VkDebugUtilsMessageTypeFlagBitsEXT;
	*	typedef struct VkDebugUtilsMessengerCallbackDataEXT {
			VkStructureType                              sType;
			const void*                                  pNext;
			VkDebugUtilsMessengerCallbackDataFlagsEXT    flags;
			const char*                                  pMessageIdName;
			int32_t                                      messageIdNumber;
			const char*                                  pMessage;
			uint32_t                                     queueLabelCount;
			const VkDebugUtilsLabelEXT*                  pQueueLabels;
			uint32_t                                     cmdBufLabelCount;
			const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
			uint32_t                                     objectCount;
			const VkDebugUtilsObjectNameInfoEXT*         pObjects;
		} VkDebugUtilsMessengerCallbackDataEXT;
	*/

	/**
		Logging callback function.
		\param messageSeverity describes the severity level of the message
		\param messageType describes the type of the message
		\param pCallbackData standard data associated with the message
		\param pUserData custom extra data which can be associated with the message
		\returns whether to end program execution
	*/
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	/**
		Make a debug messenger
		\param instance The Vulkan instance which will be debugged.
		\param dldi dynamically loads instance based dispatch functions
		\returns the created messenger
	*/
	vk::DebugUtilsMessengerEXT CreateDebugMessenger(vk::Instance& instance, vk::DispatchLoaderDynamic& dldi);

	/**
		Extract the transforms from the given bitmask.
		\param bits a bitmask describing various transforms
		\returns a vector of strings describing the transforms
	*/
	std::vector<std::string> LogTransformBits(vk::SurfaceTransformFlagsKHR bits);

	/**
		Extract the alpha composite blend modes from the given bitmask.
		\param bits a bitmask describing a combination of alpha composite options.
		\returns a vector of strings describing the options.
	*/
	std::vector<std::string> LogAlphaCompositeBits(vk::CompositeAlphaFlagsKHR bits);

	/**
		Extract image usage options.
		\param bits a bitmask describing various image usages
		\returns a vector of strings describing the image usages
	*/
	std::vector<std::string> LogImageUsageBits(vk::ImageUsageFlags bits);

	/**
		\returns a string description of the given present mode.
	*/
	std::string LogPresentMode(vk::PresentModeKHR presentMode);

	/**
		Print out the properties of the given physical device.
		\param device the physical device to investigate
	*/
	void LogDeviceProperties(const vk::PhysicalDevice& device);
}