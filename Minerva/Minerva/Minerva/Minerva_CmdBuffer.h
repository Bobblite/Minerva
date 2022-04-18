#pragma once

namespace Minerva
{
	class CommandBuffer
	{
	public:
		CommandBuffer(std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass, VkCommandBuffer _vkCommandBuffer, VkExtent2D _extent, int _index);

		inline void BindGraphicsPipeline(Minerva::Pipeline& _pipeline);
		inline void BindBuffer(Minerva::Buffer& _buffer);
		inline void BindDescriptorSet(Minerva::Pipeline& _pipeline, Minerva::DescriptorSet& _descriptorSet);
		inline void Draw(int _vertexCount, int _instanceCount, int _firstIndex, int _firstInstance);
		inline void DrawIndexed(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _firstIndex, int32_t _vertexOffset, uint32_t _firstInstance);
		inline void PushConstant(Minerva::Pipeline& _pipeline, Minerva::Shader::Type _stage, uint32_t _offset, uint32_t _size, const void* _pValue);


	private:
		std::shared_ptr<Minerva::Vulkan::CommandBuffer> m_VKCommandBufferHandle;
	};
}
