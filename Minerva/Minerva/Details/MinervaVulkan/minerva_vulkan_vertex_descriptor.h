#pragma once

namespace Minerva::Vulkan
{
	class VertexDescriptor
	{
	public:
		VertexDescriptor(std::span<Minerva::VertexDescriptor::Attribute> _attributes, uint32_t _vertexSize, Minerva::VertexDescriptor::Topology _topology);

		inline VkPrimitiveTopology GetVKTopology() const { return m_VKTopology; }
		inline VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputCreateInfo() const { return m_VKInputStageCreateInfo; }

	private:
		VkPrimitiveTopology m_VKTopology;
		VkPipelineVertexInputStateCreateInfo m_VKInputStageCreateInfo;
		std::vector<VkVertexInputBindingDescription> m_VKInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_VKInputAttributeDescriptions;

	};
}

#include "minerva_vulkan_vertex_descriptor.cpp"