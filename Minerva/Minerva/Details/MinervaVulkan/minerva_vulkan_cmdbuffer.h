#pragma once

namespace Minerva::Vulkan
{
	class CommandBuffer
	{
	public:
		CommandBuffer(std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass, VkCommandBuffer _vkCommandBuffer, VkExtent2D _extent, int _index);

		// vkCmd functions abstraction
		void BindGraphicsPipeline(std::shared_ptr<Minerva::Vulkan::Pipeline> _pipeline);
		void BindBuffer(std::shared_ptr<Minerva::Vulkan::Buffer> _buffer);
		void Draw(int _vertexCount, int _instanceCount, int _firstIndex, int _firstInstance);
		void DrawIndexed(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _firstIndex, int32_t _vertexOffset, uint32_t _firstInstance);
		void PushConstant(std::shared_ptr<Minerva::Vulkan::Pipeline> _pipeline, Minerva::Shader::Type _stage, uint32_t _offset, uint32_t _size, const void* _pValue);

	private:
		VkCommandBuffer m_VKCommandBuffer;
		std::shared_ptr<Minerva::Vulkan::Renderpass> m_VKRenderpassHandle;

		//int m_index; // Index of framebuffer the command buffer renders to
	};
}

#include "minerva_vulkan_cmdbuffer.cpp"
