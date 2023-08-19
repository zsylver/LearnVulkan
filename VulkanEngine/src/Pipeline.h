#pragma once
#include "Config.h"


namespace vkInit
{
	/**
		holds the data structures used to create a pipeline
	*/
	struct GraphicsPipelineInBundle
	{
		vk::Device device;
		std::string vertexFilepath;
		std::string fragmentFilepath;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat, depthFormat;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	};

	/**
		Used for returning the pipeline, along with associated data structures,
		after creation.
	*/
	struct GraphicsPipelineOutBundle
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};

	/**
		Make a graphics pipeline, along with renderpass and pipeline layout

		\param specification the struct holding input data, as specified at the top of the file.
		\returns the bundle of data structures created
	*/
	GraphicsPipelineOutBundle CreateGraphicsPipeline(GraphicsPipelineInBundle& specification);

	/**
		Configure the vertex input stage.

		\param bindingDescription describes the vertex inputs (ie. layouts)
		\param attributeDescriptions describes the attributes
		\returns the vertex input stage creation info

	*/
	vk::PipelineVertexInputStateCreateInfo CreateVertexInputInfo(
		const vk::VertexInputBindingDescription& bindingDescription,
		const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions);

	/**
		\returns the input assembly stage creation info
	*/
	vk::PipelineInputAssemblyStateCreateInfo CreateInputAssemblyInfo();

	/**
		Configure a programmable shader stage.

		\param shaderModule the compiled shader module
		\param stage the shader stage which the module is for
		\returns the shader stage creation info
	*/
	vk::PipelineShaderStageCreateInfo CreateShaderInfo(
		const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage);

	/**
		Create a viewport.

		\param specification holds relevant data fields
		\returns the created viewport
	*/
	vk::Viewport CreateViewport(const GraphicsPipelineInBundle& specification);

	/**
		Create a scissor rectangle.

		\param specification holds relevant data fields
		\returns the created rectangle
	*/
	vk::Rect2D CreateScissor(const GraphicsPipelineInBundle& specification);

	/**
		Configure the pipeline's viewport stage.

		\param viewport the viewport specification
		\param scissor the scissor rectangle to apply
		\returns the viewport state creation info
	*/
	vk::PipelineViewportStateCreateInfo CreateViewportState(const vk::Viewport& viewport, const vk::Rect2D& scissor);

	/**
		\returns the creation info for the configured rasterizer stage
	*/
	vk::PipelineRasterizationStateCreateInfo CreateRasterizerInfo();

	/**
		\returns the creation info for the configured multisampling stage
	*/
	vk::PipelineMultisampleStateCreateInfo CreateMultisamplingInfo();

	/**
		\returns the created color blend state
	*/
	vk::PipelineColorBlendAttachmentState CreateColorBlendAttachmentState();

	/**
		\returns the creation info for the configured color blend stage
	*/
	vk::PipelineColorBlendStateCreateInfo CreateColorBlendAttachmentStage(const vk::PipelineColorBlendAttachmentState& colorBlendAttachment);

	/**
		Make a pipeline layout, this consists mostly of describing the
		push constants and descriptor set layouts which will be used.

		\param device the logical device
		\returns the created pipeline layout
	*/
	vk::PipelineLayout CreatePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);

	/**
		Make a renderpass, a renderpass describes the subpasses involved
		as well as the attachments which will be used.

		\param device the logical device
		\param swapchainImageFormat the image format chosen for the swapchain images
		\returns the created renderpass
	*/
	vk::RenderPass CreateRenderPass(
		vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat
	);

	/**
		Make a color attachment description

		\param swapchainImageFormat the image format used by the swapchain
		\returns a description of the corresponding color attachment
	*/
	vk::AttachmentDescription CreateColorAttachment(const vk::Format& swapchainImageFormat);

	/**
		\returns Make a color attachment refernce
	*/
	vk::AttachmentReference CreateColorAttachmentReference();

	/**
		Make a depth attachment description

		\param swapchainImageFormat the image format used by the swapchain
		\returns a description of the corresponding depth attachment
	*/
	vk::AttachmentDescription CreateDepthAttachment(const vk::Format& depthFormat);

	/**
		\returns Make a depth attachment refernce
	*/
	vk::AttachmentReference CreateDepthAttachmentReference();

	/**
		Make a simple subpass.

		\param colorAttachmentRef a reference to a color attachment for the color buffer
		\returns a description of the subpass
	*/
	vk::SubpassDescription CreateSubpass(const std::vector<vk::AttachmentReference>& attachments);

	/**
		Make a simple renderpass.

		\param colorAttachment the color attachment for the color buffer
		\param subpass a description of the subpass
		\returns creation info for the renderpass
	*/
	vk::RenderPassCreateInfo CreateRenderPassInfo(const std::vector<vk::AttachmentDescription>& attachments,const vk::SubpassDescription& subpass);
}