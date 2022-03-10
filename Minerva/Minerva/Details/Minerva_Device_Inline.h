#pragma once

namespace Minerva
{
	Device::Device(const Minerva::Instance& _instance, QueueFamily _queueFamily, Type _type) :
		m_VKDeviceHandle{ nullptr }
	{
		m_VKDeviceHandle = std::make_shared<Minerva::Vulkan::Device>(_instance.GetVKInstanceHandle(), _queueFamily, _type);
	}

	inline std::shared_ptr<Minerva::Vulkan::Device> Device::GetVKDeviceHandle() const { return m_VKDeviceHandle; }

	inline Device::QueueFamily Device::GetQueueFamily() const { return m_VKDeviceHandle->GetQueueFamily(); }

	inline Device::Type Device::GetDeviceType() const { return m_VKDeviceHandle->GetDeviceType(); }
}
