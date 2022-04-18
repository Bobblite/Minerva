#pragma once
namespace Minerva
{
	Texture::Texture(Minerva::Device& _device, std::string_view _filepath) :
		m_VKTextureHandle{ nullptr }
	{
		m_VKTextureHandle = std::make_shared<Minerva::Vulkan::Texture>(_device.GetVKDeviceHandle(), _filepath);
	}
}