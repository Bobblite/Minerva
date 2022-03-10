#pragma once

namespace Minerva::Vulkan
{
	class Buffer
	{
	public:
		Buffer(std::shared_ptr<Minerva::Vulkan::Device> _device, Minerva::Buffer::Type _type, const void* _data, uint32_t _size);
		~Buffer();

		inline Minerva::Buffer::Type GetType() const { return m_Type; }
		inline VkBuffer GetVKBuffer() const { return m_VKBuffer; }
		inline VkDeviceSize GetVKSize() const { return m_VKSize; }

	private:
		// Private Handles
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;

		// Vulkan properties
		VkBuffer m_VKBuffer;
		VkDeviceMemory m_VKMemory;
		VkDeviceSize m_VKSize;

		// Minerva properties
		Minerva::Buffer::Type m_Type;

		// Helper function
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _flags, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);
	};
}

#include "minerva_vulkan_buffer.cpp"