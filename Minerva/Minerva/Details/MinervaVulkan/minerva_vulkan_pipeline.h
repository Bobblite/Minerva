#pragma once

namespace Minerva::Vulkan
{
	class Pipeline
	{
	public:
		Pipeline(std::shared_ptr<Minerva::Vulkan::Device> _device, std::shared_ptr<Minerva::Vulkan::Window> _window,
			std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass, const Minerva::Shader* _shaders, int _shaderCount, std::shared_ptr<Minerva::Vulkan::VertexDescriptor> _vertDesc);
		~Pipeline();

		inline VkPipeline GetGraphicsPipeline() const { return m_VKPipeline; }
		inline std::shared_ptr<Minerva::Vulkan::Device> GetVKDeviceHandle() const { return m_VKDeviceHandle; }
		inline std::shared_ptr<Minerva::Vulkan::Window> GetVKWindowHandle() const { return m_VKWindowHandle; }
		inline std::shared_ptr<Minerva::Vulkan::Renderpass> GetVKRenderpassHandle() const { return m_VKRenderpassHandle; }
		inline VkPipelineLayout GetVKPipelineLayout() const { return m_VKPipelineLayout; }

		void CleanupPipeline();
		void RecreatePipeline();

	private:
		// Private interface handles
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;
		std::shared_ptr<Minerva::Vulkan::Window> m_VKWindowHandle;
		std::shared_ptr<Minerva::Vulkan::Renderpass> m_VKRenderpassHandle;
		std::vector<std::shared_ptr<Minerva::Vulkan::Shader>> m_VKShaderHandles;

		// Vulkan properties
		std::vector<VkPipelineShaderStageCreateInfo> m_VKShaderStages;
		VkPipelineLayout m_VKPipelineLayout;
		VkPipeline m_VKPipeline;

		// Minerva properties
		//todo Minerva::Pipeline::Type m_Type;
		std::shared_ptr<Minerva::Vulkan::VertexDescriptor> m_VKVertexDescriptorHandle;

		// Helper function
		void CreateGraphicsPipeline();
	};
}

#include "minerva_vulkan_pipeline.cpp"
