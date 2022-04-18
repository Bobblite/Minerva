#pragma once
namespace Minerva
{
	class Texture
	{
	public:
		Texture(Minerva::Device& _device, std::string_view _filePath);

	private:
		std::shared_ptr<Minerva::Vulkan::Texture> m_VKTextureHandle;
	};
}