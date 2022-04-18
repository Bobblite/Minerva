#pragma once

namespace Minerva::Vulkan
{
	class Texture
	{
	public:
		Texture(std::shared_ptr<Minerva::Vulkan::Device> _device, std::string_view _filePath);
		~Texture();
		static VkFormat ConvertFormat(Minerva::Tools::PixelFormat::ImageFormat _format,
			Minerva::Tools::PixelFormat::ColorSpace _colorspace,
			Minerva::Tools::PixelFormat::Signedness _signedness);

		inline VkImageView GetVKImageView() const { return m_VKImageView; }
		inline VkSampler GetVKSampler() const { return m_VKSampler; }

	private:
		// Vulkan handles
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;

		VkImage m_VKImage;
		VkImageView m_VKImageView;
		VkFormat m_VKImageFormat;
		//VkBuffer m_VKImageBufferStaging;
		VkDeviceMemory m_VKImageMemory;
		VkSampler m_VKSampler;

		uint32_t m_Width;
		uint32_t m_Height;
		/*Minerva::Tools::PixelFormat::ImageFormat m_ImageFormat;
		Minerva::Tools::PixelFormat::ColorSpace m_ColorSpace;
		Minerva::Tools::PixelFormat::Signedness m_Signedness;*/
		uint32_t m_MipLevels;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);
		void TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);
		void CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height);
	};
}

#include "minerva_vulkan_texture.cpp"