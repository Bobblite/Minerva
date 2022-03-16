namespace Minerva::Vulkan
{
	Pipeline::Pipeline(std::shared_ptr<Minerva::Vulkan::Device> _device, std::shared_ptr<Minerva::Vulkan::Window> _window,
		std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass, const Minerva::Shader* _shaders, int _shaderCount, std::shared_ptr<Minerva::Vulkan::VertexDescriptor> _vertDesc) :
		m_VKDeviceHandle{ _device }, m_VKWindowHandle{ _window }, m_VKRenderpassHandle{ _renderpass }, // Private handles
		m_VKShaderHandles{}, m_VKShaderStages{}, m_VKPipelineLayout{ VK_NULL_HANDLE }, m_VKPipeline{ VK_NULL_HANDLE }, // Vulkan properties
		m_VKVertexDescriptorHandle{_vertDesc}
	{
		// Internal function to convert Minerva::Shader::Type enum to a VkShaderStageFlagBits
		constexpr auto ShaderType = []() constexpr
		{
			std::array<VkShaderStageFlagBits, static_cast<std::size_t>(Minerva::Shader::Type::ENUM_COUNT)> ShaderType{};

			ShaderType[static_cast<std::size_t>(Minerva::Shader::Type::VERTEX)] = VK_SHADER_STAGE_VERTEX_BIT;
			ShaderType[static_cast<std::size_t>(Minerva::Shader::Type::FRAGMENT)] = VK_SHADER_STAGE_FRAGMENT_BIT;
			//ShaderType[static_cast<std::size_t>(Minerva::Shader::Type::TESSELLATION)] = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			ShaderType[static_cast<std::size_t>(Minerva::Shader::Type::COMPUTE)] = VK_SHADER_STAGE_COMPUTE_BIT;

			return ShaderType;
		}();

		// Get Vulkan Shader Handles
		m_VKShaderHandles.reserve(_shaderCount);
		for (int i = 0; i < _shaderCount; ++i)
		{
			m_VKShaderHandles.emplace_back(_shaders[i].GetVKShaderHandle());
		}

		// Store Shader Stages
		m_VKShaderStages.reserve(m_VKShaderHandles.size());
		for (const auto& shader : m_VKShaderHandles)
		{
			VkPipelineShaderStageCreateInfo shaderStageInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = ShaderType[static_cast<size_t>(shader->GetShaderType())],
			.module = shader->GetVKShaderModule(),
			.pName = "main",
			};

			m_VKShaderStages.emplace_back(shaderStageInfo);
		}

		CreateGraphicsPipeline();
	}

	Pipeline::~Pipeline()
	{
		if (m_VKPipelineLayout != VK_NULL_HANDLE)
			vkDestroyPipelineLayout(m_VKDeviceHandle->GetVKDevice(), m_VKPipelineLayout, nullptr);

		if (m_VKPipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(m_VKDeviceHandle->GetVKDevice(), m_VKPipeline, nullptr);
	}

	void Pipeline::CleanupPipeline()
	{
		if (m_VKPipelineLayout != VK_NULL_HANDLE)
			vkDestroyPipelineLayout(m_VKDeviceHandle->GetVKDevice(), m_VKPipelineLayout, nullptr);

		if (m_VKPipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(m_VKDeviceHandle->GetVKDevice(), m_VKPipeline, nullptr);
	}

	void Pipeline::RecreatePipeline()
	{
		CreateGraphicsPipeline();
	}


	void Pipeline::CreateGraphicsPipeline()
	{
		//std::vector<VkDynamicState> dynamicStates{
		//	VK_DYNAMIC_STATE_VIEWPORT,
		//	VK_DYNAMIC_STATE_SCISSOR
		//};


		// Describe Vertex Input - Format of vertex data passed into vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ m_VKVertexDescriptorHandle->GetPipelineVertexInputCreateInfo() };

		// Fixed Function Stage: Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = m_VKVertexDescriptorHandle->GetVKTopology(),
			.primitiveRestartEnable = VK_FALSE
		};

		//// Set viewport
		//VkExtent2D swapExtent{ m_VKWindowHandle->GetVKSwapExtent() };
		//VkViewport viewport{
		//.x = 0.f,
		//.y = 0.f,
		//.width = (float)swapExtent.width,
		//.height = (float)swapExtent.height,
		//.minDepth = 0.f,
		//.maxDepth = 1.f
		//};

		//// Set scissors
		//VkRect2D scissors{};
		//scissors.offset = { 0, 0 };
		//scissors.extent = swapExtent;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr
		};

		// Fixed Function Stage: Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE, // Clamp between 0-1. All vertices will be rendered
			.rasterizerDiscardEnable = VK_FALSE, // Turn off rasterizer
			.polygonMode = VK_POLYGON_MODE_FILL, // Fill polygon with fragments
			.cullMode = VK_CULL_MODE_BACK_BIT, // Cull back facing
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE, // Clockwise = front facing
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f
		};

		// Multisampling: For anti-aliasing (MSAA) 
		//! [DISABLED FOR NOW]
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		// Fixed Function Stage: Color Blending - What to do with the color that Fragment Shader returns
		// Two ways: 1) Mix old and new colors. 2) Combine old and new color with bitwise operation

		// Configuration PER ATTACHED FRAMEBUFFER
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
			.blendEnable = VK_FALSE, // New color from Fragment Shader will be unmodified
			// Look up VkBlendFactor and VkBlendOp enums in specs for more info.
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};

		// Configuration for global color blending settings for all framebuffers
		VkPipelineColorBlendStateCreateInfo colorBlendAttachmentStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE, // Bitwise op blending turned off
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachmentState
		};
		colorBlendAttachmentStateCreateInfo.blendConstants[0] = 0.0f;
		colorBlendAttachmentStateCreateInfo.blendConstants[1] = 0.0f;
		colorBlendAttachmentStateCreateInfo.blendConstants[2] = 0.0f;
		colorBlendAttachmentStateCreateInfo.blendConstants[3] = 0.0f;

		// Dynamic States - To change state previously specified without recreating pipeline
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		// Creating a Pipeline Layout - To specify Uniforms or Descriptor Sets to Shaders

		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = 64;
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = 0,
			.pSetLayouts = nullptr,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &pushConstant
		};

		if (auto VkErr{ vkCreatePipelineLayout(m_VKDeviceHandle->GetVKDevice(), &pipelineLayoutCreateInfo, nullptr, &m_VKPipelineLayout) }; VkErr)
		{
			Logger::Log_Error("Unable to create pipeline. Failed to create Pipeline Layout");
			throw std::runtime_error("Unable to create pipeline. Failed to create Pipeline Layout");
		}

		// Describe Graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = 2, // Shader stages count
			.pStages = m_VKShaderStages.data(), // Container of shader stages
			.pVertexInputState = &vertexInputInfo, // Vertex input state
			.pInputAssemblyState = &inputAssemblyCreateInfo, // Assembly stage state
			.pViewportState = &viewportStateCreateInfo, // Viewport state when rasterization enabled
			.pRasterizationState = &rasterizerCreateInfo, // Rasterization state
			.pMultisampleState = &multisamplingCreateInfo, // Multisampling state when rasterization enabled
			.pDepthStencilState = nullptr, // Depth or stencil attachment
			.pColorBlendState = &colorBlendAttachmentStateCreateInfo, // Color blending stage state
			.pDynamicState = &dynamicState, // Determines what properties are dynamic and CAN be changed independently of pipeline state
			.layout = m_VKPipelineLayout, // Uniform/Descriptor set binding
			.renderPass = m_VKRenderpassHandle->GetVKRenderPass(), // Renderpass describing environment which pipeline can be used.
			.subpass = 0, // Index of the subpass inside renderpass where pipeline will be used
			.basePipelineHandle = VK_NULL_HANDLE, // Base pipeline handle
			.basePipelineIndex = -1 // Base pipeline index
		};

		// Create Graphics Pipeline
		if (auto VkErr{ vkCreateGraphicsPipelines(m_VKDeviceHandle->GetVKDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_VKPipeline)}; VkErr)
		{
			Logger::Log_Error("Failed to create Graphics Pipeline. vkCreateGraphicsPipeline failed.");
			throw std::runtime_error("Failed to create Graphics Pipeline. vkCreateGraphicsPipeline failed.");
		}
	}
}
