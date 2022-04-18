#pragma once

namespace Minerva
{
	CommandBuffer::CommandBuffer(std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass, VkCommandBuffer _vkCommandBuffer, VkExtent2D _extent, int _index) :
		m_VKCommandBufferHandle{ nullptr }
	{
		m_VKCommandBufferHandle = std::make_shared<Minerva::Vulkan::CommandBuffer>(_renderpass, _vkCommandBuffer, _extent, _index);
	}

	inline void CommandBuffer::BindGraphicsPipeline(Minerva::Pipeline& _pipeline)
	{
		m_VKCommandBufferHandle->BindGraphicsPipeline(_pipeline.GetVKPipelineHandle());
	}

	inline void CommandBuffer::BindBuffer(Minerva::Buffer& _buffer)
	{
		m_VKCommandBufferHandle->BindBuffer(_buffer.GetVKBufferHandle());
	}

	inline void CommandBuffer::BindDescriptorSet(Minerva::Pipeline& _pipeline, Minerva::DescriptorSet& _descriptorSet)
	{
		m_VKCommandBufferHandle->BindDescriptorSet(_pipeline.GetVKPipelineHandle(), _descriptorSet.GetVKDescriptorSetHandle());
	}

	inline void CommandBuffer::Draw(int _vertexCount, int _instanceCount, int _firstIndex, int _firstInstance)
	{
		m_VKCommandBufferHandle->Draw(_vertexCount, _instanceCount, _firstIndex, _firstInstance);
	}

	inline void CommandBuffer::DrawIndexed(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _firstIndex, int32_t _vertexOffset, uint32_t _firstInstance)
	{
		m_VKCommandBufferHandle->DrawIndexed(_indexCount, _instanceCount, _firstIndex, _vertexOffset, _firstInstance);
	}

	inline void CommandBuffer::PushConstant(Minerva::Pipeline& _pipeline, Minerva::Shader::Type _stage, uint32_t _offset, uint32_t _size, const void* _pValue)
	{
		m_VKCommandBufferHandle->PushConstant(_pipeline.GetVKPipelineHandle(), _stage, _offset, _size, _pValue);
	}

}
