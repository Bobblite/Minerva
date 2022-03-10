#pragma once

namespace Minerva
{
	VertexDescriptor::VertexDescriptor(std::span<Attribute> _attributes, uint32_t _vertexSize, Topology _topology) :
		m_VKVertexDescriptorHandle{ nullptr }
	{
		m_VKVertexDescriptorHandle = std::make_shared<Minerva::Vulkan::VertexDescriptor>(_attributes, _vertexSize, _topology);
	}

	inline std::shared_ptr<Minerva::Vulkan::VertexDescriptor> VertexDescriptor::GetVKVertexDescriptorHandle() const { return m_VKVertexDescriptorHandle; }
}
