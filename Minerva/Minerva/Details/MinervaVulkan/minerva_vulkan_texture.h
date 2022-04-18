#pragma once

namespace Minerva::Vulkan
{
	class Texture
	{
	public:
		Texture(std::shared_ptr<Minerva::Vulkan::Device> _device, std::string_view _filePath);

	private:
		VkImage m_VKImage;
		VkImageView m_VKImageView;
		VkDeviceMemory m_VKImageMemory;
		VkSampler m_VKSampler;
	};
}

#include "minerva_vulkan_texture.cpp"