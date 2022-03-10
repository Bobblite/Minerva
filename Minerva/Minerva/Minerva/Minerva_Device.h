#pragma once

namespace Minerva
{
	class Device
	{
	public:
		enum class QueueFamily : uint8_t
		{
			RENDER_AND_SWAP
			, RENDER_ONLY
			, COMPUTE
			, COPY
			, ENUM_COUNT
		};

		enum class Type : uint8_t
		{
			ANY_GPU
			, DISCRETE_ONLY
			, NON_DISCRETE_ONLY
		};

		Device(const Minerva::Instance& _instance, QueueFamily _queueFamily, Type _type);

		inline std::shared_ptr<Minerva::Vulkan::Device> GetVKDeviceHandle() const;
		inline QueueFamily GetQueueFamily() const;
		inline Type GetDeviceType() const;

		//inline void CopyBuffer(Minerva::Buffer& _src, Minerva::Buffer& _dst);

	private:
		// Private interface Handle
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;
	};
}
