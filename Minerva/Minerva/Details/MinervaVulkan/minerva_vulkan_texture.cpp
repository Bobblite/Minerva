namespace Minerva::Vulkan
{
	Texture::Texture(std::shared_ptr<Minerva::Vulkan::Device> _device, std::string_view _filePath) :
		m_VKImage{ VK_NULL_HANDLE }, m_VKImageView{ VK_NULL_HANDLE }, m_VKImageMemory{ VK_NULL_HANDLE }, m_VKSampler{ VK_NULL_HANDLE }
	{

	}
}