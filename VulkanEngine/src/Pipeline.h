#pragma once
#include "Config.h"


namespace vkInit
{
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

	class PipelineBuilder 
	{
	public:

		PipelineBuilder(vk::Device device);
		~PipelineBuilder();

		void Reset();

		/**
			Configure the vertex input stage.

			\param bindingDescription describes the vertex inputs (ie. layouts)
			\param attributeDescriptions describes the attributes
			\returns the vertex input stage creation info

		*/
		void SpecifyVertexFormat(
			vk::VertexInputBindingDescription bindingDescription,
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions);

		void SpecifyVertexShader(const char* filename);

		void SpecifyFragmentShader(const char* filename);

		void SpecifySwapChainExtent(vk::Extent2D screen_size);

		void SpecifyDepthAttachment(const vk::Format& depthFormat, uint32_t attachment_index);

		void ClearDepthAttachment();

		void AddColorAttachment(const vk::Format& format, uint32_t attachment_index);

		void SetOverwriteMode(bool mode);

		/**
			Make a graphics pipeline, along with renderpass and pipeline layout

			\param specification the struct holding input data, as specified at the top of the file.
			\returns the bundle of data structures created
		*/
		GraphicsPipelineOutBundle Build();

		void AddDescriptorSetLayout(vk::DescriptorSetLayout descriptorSetLayout);

		void ResetDescriptorSetLayouts();

	private:
		vk::Device device;
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};

		vk::VertexInputBindingDescription bindingDescription;
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		vk::ShaderModule vertexShader = nullptr, fragmentShader = nullptr;
		vk::PipelineShaderStageCreateInfo vertexShaderInfo, fragmentShaderInfo;

		vk::Extent2D swapchainExtent;
		vk::Viewport viewport = {};
		vk::Rect2D scissor = {};
		vk::PipelineViewportStateCreateInfo viewportState = {};

		vk::PipelineRasterizationStateCreateInfo rasterizer = {};

		vk::PipelineDepthStencilStateCreateInfo depthState;
		std::unordered_map<uint32_t, vk::AttachmentDescription> attachmentDescriptions;
		std::unordered_map<uint32_t, vk::AttachmentReference> attachmentReferences;
		std::vector<vk::AttachmentDescription> flattenedAttachmentDescriptions;
		std::vector<vk::AttachmentReference> flattenedAttachmentReferences;

		vk::PipelineMultisampleStateCreateInfo multisampling = {};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		vk::PipelineColorBlendStateCreateInfo colorBlending = {};

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		bool overwrite;

		void ResetVertexFormat();

		void ResetShaderModules();

		void ResetRenderPassAttachments();

		/**
			Make an attachment description

			\param format the image format for the underlying resource
			\param finalLayout the expected final layout after implicit transition (acquisition)
			\returns a description of the corresponding attachment
		*/
		vk::AttachmentDescription CreateRenderPassAttachment(
			const vk::Format& format, vk::AttachmentLoadOp loadOp,
			vk::AttachmentStoreOp storeOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);

		/**
			\returns Make a renderpass attachment reference
		*/
		vk::AttachmentReference CreateAttachmentReference(
			uint32_t attachment_index, vk::ImageLayout layout);

		/**
			set up the input assembly stage
		*/
		void ConfigureInputAssembly();

		/**
			Configure a programmable shader stage.

			\param shaderModule the compiled shader module
			\param stage the shader stage which the module is for
			\returns the shader stage creation info
		*/
		vk::PipelineShaderStageCreateInfo CreateShaderInfo(
			const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage);

		/**
			Configure the pipeline's viewport stage.

			\param viewport the viewport specification
			\param scissor the scissor rectangle to apply
			\returns the viewport state creation info
		*/
		vk::PipelineViewportStateCreateInfo CreateViewportState();

		/**
			sets the creation info for the configured rasterizer stage
		*/
		void CreateRasterizerInfo();

		/**
			configures the multisampling stage
		*/
		void ConfigureMultisampling();

		/**
			configures the color blending stage
		*/
		void ConfigureColorBlending();

		/**
			Make a pipeline layout, this consists mostly of describing the
			push constants and descriptor set layouts which will be used.

			\returns the created pipeline layout
		*/
		vk::PipelineLayout CreatePipelineLayout();

		/**
			Make a renderpass, a renderpass describes the subpasses involved
			as well as the attachments which will be used.

			\returns the created renderpass
		*/
		vk::RenderPass CreateRenderPass();

		/**
			Make a simple subpass.

			\param colorAttachmentRef a reference to a color attachment for the color buffer
			\returns a description of the subpass
		*/
		vk::SubpassDescription CreateSubpass(
			const std::vector<vk::AttachmentReference>& attachments
		);

		/**
			Make a simple renderpass.

			\param colorAttachment the color attachment for the color buffer
			\param subpass a description of the subpass
			\returns creation info for the renderpass
		*/
		vk::RenderPassCreateInfo CreateRenderPassInfo(
			const std::vector<vk::AttachmentDescription>& attachments,
			const vk::SubpassDescription& subpass
		);
	};
}