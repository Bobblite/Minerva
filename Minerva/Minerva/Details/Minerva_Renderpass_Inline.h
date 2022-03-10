#pragma once

namespace Minerva
{
	Renderpass::Renderpass(const Minerva::Device& _device, const Minerva::Window& _window, float* _clearColor) :
		m_VKRenderpassHandle{ nullptr }
	{
		m_VKRenderpassHandle = std::make_shared<Minerva::Vulkan::Renderpass>(_device.GetVKDeviceHandle(), _window.GetVKWindowHandle(), _clearColor);
	}

	inline void Renderpass::RecreateRenderpass() { m_VKRenderpassHandle->RecreateRenderpass(); }
	inline void Renderpass::CleanupRenderpass() { m_VKRenderpassHandle->CleanupRenderpass(); }
}
