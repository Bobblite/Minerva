#pragma once

namespace Minerva
{
	Buffer::Buffer(Minerva::Device& _device, Type _type, const void* _data, uint32_t _size) :
		m_VKBufferHandle{ nullptr }
	{
		m_VKBufferHandle = std::make_shared<Minerva::Vulkan::Buffer>(_device.GetVKDeviceHandle(), _type, _data, _size);
	}

	inline std::shared_ptr<Minerva::Vulkan::Buffer> Buffer::GetVKBufferHandle() const
	{
		return m_VKBufferHandle;
	}

	inline VkBuffer Buffer::GetVKBuffer() const { return m_VKBufferHandle->GetVKBuffer(); }
}
