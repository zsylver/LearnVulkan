#include "Pipeline.h"
#include "Shaders.h"
#include "RenderStructs.h"
#include "Mesh.h"

/**
	Make a graphics pipeline, along with renderpass and pipeline layout

	\param specification the struct holding input data, as specified at the top of the file.
	\returns the bundle of data structures created
	*/
vkInit::GraphicsPipelineOutBundle vkInit::CreateGraphicsPipeline(GraphicsPipelineInBundle& specification)
{
	/*
	* Build and return a graphics pipeline based on the given info.
	*/

	//The info for the graphics pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.flags = vk::PipelineCreateFlags();

	//Shader stages, to be populated later
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

	//Vertex Input
	vk::VertexInputBindingDescription bindingDescription = vkMesh::GetPosColorBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = vkMesh::GetPosColorAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = CreateVertexInputInfo(bindingDescription, attributeDescriptions);
	pipelineInfo.pVertexInputState = &vertexInputInfo;

	//Input Assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = CreateInputAssemblyInfo();
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

	//Vertex Shader
	std::cout << "Creating vertex shader module" << std::endl;
	vk::ShaderModule vertexShader = vkUtil::CreateModule(
		specification.vertexFilepath, specification.device
	);
	vk::PipelineShaderStageCreateInfo vertexShaderInfo = CreateShaderInfo(vertexShader, vk::ShaderStageFlagBits::eVertex);
	shaderStages.push_back(vertexShaderInfo);

	//Viewport and Scissor
	vk::Viewport viewport = CreateViewport(specification);
	vk::Rect2D scissor = CreateScissor(specification);
	vk::PipelineViewportStateCreateInfo viewportState = CreateViewportState(viewport, scissor);
	pipelineInfo.pViewportState = &viewportState;

	//Rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizer = CreateRasterizerInfo();
	pipelineInfo.pRasterizationState = &rasterizer;

	//Fragment Shader
	std::cout << "Creating fragment shader module" << std::endl;

	vk::ShaderModule fragmentShader = vkUtil::CreateModule(
		specification.fragmentFilepath, specification.device
	);
	vk::PipelineShaderStageCreateInfo fragmentShaderInfo = CreateShaderInfo(fragmentShader, vk::ShaderStageFlagBits::eFragment);
	shaderStages.push_back(fragmentShaderInfo);
	//Now both shaders have been made, we can declare them to the pipeline info
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();

	//Depth-Stencil
	vk::PipelineDepthStencilStateCreateInfo depthState;
	depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
	depthState.depthTestEnable = true;
	depthState.depthWriteEnable = true;
	depthState.depthCompareOp = vk::CompareOp::eLess;
	depthState.depthBoundsTestEnable = false;
	depthState.stencilTestEnable = false;
	pipelineInfo.pDepthStencilState = &depthState;

	//Multisampling
	vk::PipelineMultisampleStateCreateInfo multisampling = CreateMultisamplingInfo();
	pipelineInfo.pMultisampleState = &multisampling;

	//Color Blend
	vk::PipelineColorBlendAttachmentState colorBlendAttachment = CreateColorBlendAttachmentState();
	vk::PipelineColorBlendStateCreateInfo colorBlending = CreateColorBlendAttachmentStage(colorBlendAttachment);
	pipelineInfo.pColorBlendState = &colorBlending;

	//Pipeline Layout
	std::cout << "Creating Pipeline Layout" << std::endl;
	vk::PipelineLayout pipelineLayout = CreatePipelineLayout(specification.device, specification.descriptorSetLayouts);
	pipelineInfo.layout = pipelineLayout;

	//Renderpass
	std::cout << "Creating RenderPass" << std::endl;
	vk::RenderPass renderpass = CreateRenderPass(
		specification.device, specification.swapchainImageFormat,
		specification.depthFormat
	);
	pipelineInfo.renderPass = renderpass;
	pipelineInfo.subpass = 0;

	//Extra stuff
	pipelineInfo.basePipelineHandle = nullptr;

	//Make the Pipeline
	std::cout << "Creating Graphics Pipeline" << std::endl;
	vk::Pipeline graphicsPipeline;
	try
	{
		graphicsPipeline = (specification.device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
	}
	catch (vk::SystemError err)
	{
		throw std::runtime_error("Failed to create Pipeline");
	}

	GraphicsPipelineOutBundle output;
	output.layout = pipelineLayout;
	output.renderpass = renderpass;
	output.pipeline = graphicsPipeline;

	//Finally clean up by destroying shader modules
	specification.device.destroyShaderModule(vertexShader);
	specification.device.destroyShaderModule(fragmentShader);

	return output;
}

vk::PipelineVertexInputStateCreateInfo vkInit::CreateVertexInputInfo(
	const vk::VertexInputBindingDescription& bindingDescription,
	const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions)
{
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	return vertexInputInfo;
}

vk::PipelineInputAssemblyStateCreateInfo vkInit::CreateInputAssemblyInfo()
{
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

	return inputAssemblyInfo;
}

vk::PipelineShaderStageCreateInfo vkInit::CreateShaderInfo(const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage)
{
	vk::PipelineShaderStageCreateInfo shaderInfo = {};
	shaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	shaderInfo.stage = stage;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
	return shaderInfo;
}

vk::Viewport vkInit::CreateViewport(const vkInit::GraphicsPipelineInBundle& specification)
{
	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)specification.swapchainExtent.width;
	viewport.height = (float)specification.swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	return viewport;
}

vk::Rect2D vkInit::CreateScissor(const vkInit::GraphicsPipelineInBundle& specification)
{
	vk::Rect2D scissor = {};
	scissor.offset.x = 0.0f;
	scissor.offset.y = 0.0f;
	scissor.extent = specification.swapchainExtent;

	return scissor;
}

vk::PipelineViewportStateCreateInfo vkInit::CreateViewportState(const vk::Viewport& viewport, const vk::Rect2D& scissor)
{
	vk::PipelineViewportStateCreateInfo viewportState = {};
	viewportState.flags = vk::PipelineViewportStateCreateFlags();
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	return viewportState;
}

vk::PipelineRasterizationStateCreateInfo vkInit::CreateRasterizerInfo()
{
	vk::PipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.

	return rasterizer;
}

vk::PipelineMultisampleStateCreateInfo vkInit::CreateMultisamplingInfo()
{
	vk::PipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	return multisampling;
}

vk::PipelineColorBlendAttachmentState vkInit::CreateColorBlendAttachmentState()
{
	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
	colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

	return colorBlendAttachment;
}

vk::PipelineColorBlendStateCreateInfo vkInit::CreateColorBlendAttachmentStage(const vk::PipelineColorBlendAttachmentState& colorBlendAttachment)
{
	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	return colorBlending;
}

vk::PipelineLayout vkInit::CreatePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
	/*
	typedef struct VkPipelineLayoutCreateInfo {
		VkStructureType                 sType;
		const void*                     pNext;
		VkPipelineLayoutCreateFlags     flags;
		uint32_t                        setLayoutCount;
		const VkDescriptorSetLayout*    pSetLayouts;
		uint32_t                        pushConstantRangeCount;
		const VkPushConstantRange*      pPushConstantRanges;
	} VkPipelineLayoutCreateInfo;
	*/

	vk::PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.flags = vk::PipelineLayoutCreateFlags();

	layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pSetLayouts = descriptorSetLayouts.data();

	layoutInfo.pushConstantRangeCount = 0;

	try
	{
		return device.createPipelineLayout(layoutInfo);
	}
	catch (vk::SystemError err)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

vk::RenderPass vkInit::CreateRenderPass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat)
{
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentReference> attachmentReferences;

	//Color Buffer
	attachments.push_back(CreateColorAttachment(swapchainImageFormat));
	attachmentReferences.push_back(CreateColorAttachmentReference());

	//Depth Buffer
	attachments.push_back(CreateDepthAttachment(depthFormat));
	attachmentReferences.push_back(CreateDepthAttachmentReference());

	//Renderpasses are broken down into subpasses, there's always at least one.
	vk::SubpassDescription subpass = CreateSubpass(attachmentReferences);

	//Now create the renderpass
	vk::RenderPassCreateInfo renderpassInfo = CreateRenderPassInfo(attachments, subpass);
	try
	{
		return device.createRenderPass(renderpassInfo);
	}
	catch (vk::SystemError err)
	{
		throw std::runtime_error("Failed to create renderpass!");
	}

}

vk::AttachmentDescription vkInit::CreateColorAttachment(const vk::Format& swapchainImageFormat) {

	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.flags = vk::AttachmentDescriptionFlags();
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	return colorAttachment;
}

vk::AttachmentReference vkInit::CreateColorAttachmentReference() {

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	return colorAttachmentRef;
}

vk::AttachmentDescription vkInit::CreateDepthAttachment(const vk::Format& depthFormat) {

	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.flags = vk::AttachmentDescriptionFlags();
	depthAttachment.format = depthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	return depthAttachment;
}

vk::AttachmentReference vkInit::CreateDepthAttachmentReference() {

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	return depthAttachmentRef;
}

vk::SubpassDescription vkInit::CreateSubpass(
	const std::vector<vk::AttachmentReference>& attachments) 
{
	vk::SubpassDescription subpass = {};
	subpass.flags = vk::SubpassDescriptionFlags();
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachments[0];
	subpass.pDepthStencilAttachment = &attachments[1];

	return subpass;
}

vk::RenderPassCreateInfo vkInit::CreateRenderPassInfo(
	const std::vector<vk::AttachmentDescription>& attachments,
	const vk::SubpassDescription& subpass) 
{
	vk::RenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.flags = vk::RenderPassCreateFlags();
	renderpassInfo.attachmentCount = attachments.size();
	renderpassInfo.pAttachments = attachments.data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;

	return renderpassInfo;
}
