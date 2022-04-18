#pragma once

namespace Minerva::Vulkan
{
	class DescriptorSet
	{
	public:
		DescriptorSet(std::shared_ptr<Minerva::Vulkan::Device> _device, std::span<Minerva::DescriptorSet::Layout> _layouts);
		~DescriptorSet();

		inline VkDescriptorSetLayout GetVKDescriptorSetLayout() const { return m_VKDescriptorSetLayout; }
		inline VkDescriptorSet GetVKDescriptorSet() const { return m_VKDescriptorSet; }
		
		void Update(const Minerva::DescriptorSet::Layout& _layout, std::span<std::shared_ptr<Minerva::Vulkan::Texture>> _textures);

	private:
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;

		VkDescriptorSet m_VKDescriptorSet;
		VkDescriptorSetLayout m_VKDescriptorSetLayout;
	};
}

#include "minerva_vulkan_descriptorset.cpp"
