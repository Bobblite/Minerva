#pragma once

namespace Minerva::Vulkan
{
	class DescriptorSet
	{
	public:
		DescriptorSet(std::shared_ptr<Minerva::Vulkan::Device> _device, std::span<Minerva::DescriptorSet::Layout> _layouts);
		~DescriptorSet();

		//todo Update(const Layout& _layout, void* _data, uint32_t _size);

	private:
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;

		VkDescriptorSet m_VKDescriptorSet;
		VkDescriptorSetLayout m_VKDescriptorSetLayout;
	};
}

#include "minerva_vulkan_descriptorset.cpp"
