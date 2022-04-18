#pragma once

namespace Minerva
{
	DescriptorSet::DescriptorSet(Minerva::Device& _device, std::span<Layout> _layouts) :
		m_VKDescriptorSetHandle{ nullptr }
	{
		m_VKDescriptorSetHandle = std::make_shared<Minerva::Vulkan::DescriptorSet>(_device.GetVKDeviceHandle(), _layouts);
	}

	DescriptorSet::~DescriptorSet() {}
	
	inline std::shared_ptr<Minerva::Vulkan::DescriptorSet> DescriptorSet::GetVKDescriptorSetHandle() const
	{
		return m_VKDescriptorSetHandle;
	}

	inline void DescriptorSet::Update(const Layout& _layout, std::span<Minerva::Texture> _textures)
	{
		// Create a container of Vulkan texture handles
		std::vector<std::shared_ptr<Minerva::Vulkan::Texture>> textures(_textures.size());
		for (int i{ 0 }; i < _textures.size(); ++i)
			textures[i] = _textures[i].GetVKTextureHandle();

		m_VKDescriptorSetHandle->Update(_layout, textures);
	}
}
