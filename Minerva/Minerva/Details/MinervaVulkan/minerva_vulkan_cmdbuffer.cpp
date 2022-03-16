namespace Minerva::Vulkan
{
	CommandBuffer::CommandBuffer(std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass, VkCommandBuffer _vkCommandBuffer, VkExtent2D _extent, int _index) :
		m_VKCommandBuffer{ _vkCommandBuffer }, m_VKRenderpassHandle{ _renderpass }
	{
		// Describe command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Begin command buffer
		if (vkBeginCommandBuffer(m_VKCommandBuffer, &beginInfo) != VK_SUCCESS) {
			Logger::Log_Error("Failed to being recording Command Buffer.");
			throw std::runtime_error("Failed to being recording Command Buffer.");
		}

		// Describe Render pass being info
		VkClearValue clearValue{ m_VKRenderpassHandle->GetVkClearValue() };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_VKRenderpassHandle->GetVKRenderPass();
		renderPassInfo.framebuffer = m_VKRenderpassHandle->GetVKFramebuffers()[_index];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		// Begin render pass
		vkCmdBeginRenderPass(m_VKCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::BindGraphicsPipeline(std::shared_ptr<Minerva::Vulkan::Pipeline> _pipeline)
	{
		if (_pipeline->GetGraphicsPipeline() == VK_NULL_HANDLE)
			Logger::Log_Error("Unable to bind graphics pipeline. Graphics Pipeline does not exist.");
		vkCmdBindPipeline(m_VKCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->GetGraphicsPipeline());
	}

	void CommandBuffer::BindBuffer(std::shared_ptr<Minerva::Vulkan::Buffer> _buffer)
	{
		switch (_buffer->GetType())
		{
		case Minerva::Buffer::Type::VERTEX:
		{
			std::array<VkBuffer, 1> vertexBuffers = { _buffer->GetVKBuffer() };
			std::array<VkDeviceSize, 1> deviceOffsets = { 0 };
			vkCmdBindVertexBuffers(m_VKCommandBuffer, 0, 1, vertexBuffers.data(), deviceOffsets.data());
		}break;

		case Minerva::Buffer::Type::INDEX:
		{
			vkCmdBindIndexBuffer(m_VKCommandBuffer, _buffer->GetVKBuffer(), 0, VK_INDEX_TYPE_UINT16);
		} break;
		}

	}

	void CommandBuffer::Draw(int _vertexCount, int _instanceCount, int _firstIndex, int _firstInstance)
	{
		vkCmdDraw(m_VKCommandBuffer, _vertexCount, _instanceCount, _firstIndex, _firstInstance);
	}

	void CommandBuffer::DrawIndexed(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _firstIndex, int32_t _vertexOffset, uint32_t _firstInstance)
	{
		// Setup viewport
		VkExtent2D frambufferExtent{ m_VKRenderpassHandle->GetFramebufferExtent() };
		VkViewport viewport{
			.x = 0.f,
			.y = 0.f,
			.width = static_cast<float>(frambufferExtent.width),
			.height = static_cast<float>(frambufferExtent.height),
			.minDepth = 0.f,
			.maxDepth = 1.f
		};

		// Setup clip scissors
		VkRect2D scissors{
			.offset = {0, 0},
			.extent = frambufferExtent,
		};

		// Set dynamic states
		vkCmdSetViewport(m_VKCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_VKCommandBuffer, 0, 1, &scissors);

		// Draw
		vkCmdDrawIndexed(m_VKCommandBuffer, _indexCount, _instanceCount, _firstIndex, _vertexOffset, _firstInstance);
	}

	void CommandBuffer::PushConstant(std::shared_ptr<Minerva::Vulkan::Pipeline> _pipeline, Minerva::Shader::Type _stage, uint32_t _offset, uint32_t _size, const void* _pValue)
	{
		auto ShaderStage = [](auto ShaderStage) constexpr
		{
			switch (ShaderStage)
			{
			case Minerva::Shader::Type::VERTEX:				return VK_SHADER_STAGE_VERTEX_BIT;
			case Minerva::Shader::Type::FRAGMENT:			return VK_SHADER_STAGE_FRAGMENT_BIT;
				//case Minerva::Buffer::Type::UNIFORM:               return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			}
		}(_stage);

		vkCmdPushConstants(m_VKCommandBuffer, _pipeline->GetVKPipelineLayout(), ShaderStage, _offset, _size, _pValue);
	}
}
