#pragma once

namespace Minerva
{
	DescriptorSet::DescriptorSet(Minerva::Device& _device, std::span<Layout> _layouts) :
		m_VKDescriptorSetHandle{ nullptr }
	{
		m_VKDescriptorSetHandle = std::make_shared<Minerva::Vulkan::DescriptorSet>(_device.GetVKDeviceHandle(), _layouts);
	}

	DescriptorSet::~DescriptorSet() {}
}
