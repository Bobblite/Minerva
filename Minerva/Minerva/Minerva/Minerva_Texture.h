#pragma once
namespace Minerva
{
	class Texture
	{
	public:
		Texture(Minerva::Device& _device, std::string_view _filePath);

		inline std::shared_ptr<Minerva::Vulkan::Texture> GetVKTextureHandle() const;

	private:
		std::shared_ptr<Minerva::Vulkan::Texture> m_VKTextureHandle;
	};
}