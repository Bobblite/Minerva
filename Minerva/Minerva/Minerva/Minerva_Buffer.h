#pragma once

namespace Minerva
{
	class Buffer
	{
	public:
		enum class Type : uint8_t
		{
			VERTEX,
			INDEX,
			UNIFORM,
			TRANSFER_SRC,
		};

		Buffer(Minerva::Device& _device, Type _type, const void* _data, uint32_t _size);
		inline std::shared_ptr<Minerva::Vulkan::Buffer> GetVKBufferHandle() const;
		inline VkBuffer GetVKBuffer() const;

	private:
		std::shared_ptr<Minerva::Vulkan::Buffer> m_VKBufferHandle;
	};
}
