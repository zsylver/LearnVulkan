#pragma once
#include "Config.h"

//namespace for creation fucntions/definitions etc.
namespace vkInit
{
	bool CheckSupport(std::vector<const char*>& extensions, std::vector<const char*>& layers, bool debug)
	{
		//check extension support
		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();


		// Prints all available supported extensions
		//if (debug) 
		//{
		//	std::cout << "Device can support the following extensions:\n";
		//	for (vk::ExtensionProperties supportedExtension : supportedExtensions) 
		//	{
		//		std::cout << '\t' << supportedExtension.extensionName << '\n';
		//	}
		//}

		bool found;
		for (const char* extension : extensions)
		{
			found = false;
			for (vk::ExtensionProperties supportedExtension : supportedExtensions)
			{
				if (strcmp(extension, supportedExtension.extensionName) == 0) {
					found = true;
					if (debug)
					{
						std::cout << "Extension \"" << extension << "\" is supported!\n";
					}
				}
			}
			if (!found)
			{
				if (debug)
				{
					std::cout << "Extension \"" << extension << "\" is not supported!\n";
				}
				return false;
			}
		}

		//check layer support
		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

		// Prints all available supported layers

		//if (debug) 
		//{
		//	std::cout << "Device can support the following layers:\n";
		//	for (vk::LayerProperties supportedLayer : supportedLayers) 
		//	{
		//		std::cout << '\t' << supportedLayer.layerName << '\n';
		//	}
		//}

		std::cout << "Layers to be requested:\n";

		for (const char* layer : layers)
		{
			std::cout << "\t\"" << layer << "\"\n";
		}


		for (const char* layer : layers)
		{
			found = false;
			for (vk::LayerProperties supportedLayer : supportedLayers)
			{
				if (strcmp(layer, supportedLayer.layerName) == 0)
				{
					found = true;
					if (debug)
					{
						std::cout << "Layer \"" << layer << "\" is supported!\n";
					}
				}
			}
			if (!found)
			{
				if (debug)
				{
					std::cout << "Layer \"" << layer << "\" is not supported!\n";
				}
				return false;
			}
		}

		return true;
	}

	/**
		Create a Vulkan instance.
		\param debug whether the system is being run in debug mode.
		\param applicationName the name of the application.
		\returns the instance created.
	*/
	/**
	Create a Vulkan instance.
	\param debug whether the system is being run in debug mode.
	\param applicationName the name of the application.
	\returns the instance created.
	*/
	vk::Instance CreateInstance(bool debug, const char* appName)
	{
		if (debug)
		{
			std::cout << "Creating a vulkan instance...\n";
		}

		/*
		* An instance stores all per-application state info, it is a vulkan handle
		* (An opaque integer or pointer value used to refer to a Vulkan object)
		* side note: in the vulkan.hpp binding it's a wrapper class around a handle
		*
		* from vulkan_core.h:
		* VK_DEFINE_HANDLE(VkInstance)
		*
		* from vulkan_handles.hpp:
		* class Instance {
		* ...
		* }
		*/

		/*
		* We can scan the system and check which version it will support up to,
		* as of vulkan 1.1
		*
		* VkResult vkEnumerateInstanceVersion(
		uint32_t*                                   pApiVersion);
		*/

		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);

		if (debug)
		{
			std::cout
				<< "System can support vulkan Variant: " << VK_API_VERSION_VARIANT(version)
				<< ", Major: " << VK_API_VERSION_MAJOR(version)
				<< ", Minor: " << VK_API_VERSION_MINOR(version)
				<< ", Patch: " << VK_API_VERSION_PATCH(version) << '\n';
		}

		/*
		* We can then either use this version
		* We should just be sure to set the patch to 0 for best compatibility/stability
		*/
		version &= ~(0xFFFU);

		/*
		* Or drop down to an earlier version to ensure compatibility with more devices
		* VK_MAKE_API_VERSION(variant, major, minor, patch)
		*/
		version = VK_MAKE_API_VERSION(0, 1, 0, 0);

		vk::ApplicationInfo appInfo
		{
			appName,
			version,
			"LearnVulkanEngine",
			version,
			version
		};

		/*
		* Everything with Vulkan is "opt-in", so we need to query which extensions glfw needs
		* in order to interface with vulkan.
		*/
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		std::vector<const char*> layers;


		if (debug)
		{
			//In order to hook in a custom validation callback
			extensions.push_back("VK_EXT_debug_utils");

			// Print all requested extensions
			std::cout << "Extensions to be requested:\n";

			for (const char* extensionName : extensions)
			{
				std::cout << "\t\"" << extensionName << "\"\n";
			}

			layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		if (!CheckSupport(extensions, layers, debug))
		{
			return nullptr;
		}

		vk::InstanceCreateInfo createInfo
		{
			vk::InstanceCreateFlags(),
			&appInfo,
			static_cast<uint32_t>(layers.size()), layers.data(), // enabled layers
			static_cast<uint32_t>(extensions.size()), extensions.data() // enabled extensions
		};

		try
		{
			return vk::createInstance(createInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug)
				std::cout << "Failed to create a Vulkan Instance!" << std::endl;

			return nullptr;
		}
	}
}