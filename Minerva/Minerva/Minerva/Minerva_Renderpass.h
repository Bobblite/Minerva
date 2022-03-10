#pragma once

namespace Minerva
{
	class Renderpass
	{
	public:
		Renderpass(const Minerva::Device& _device, const Minerva::Window& _window, float* _clearColor);

		inline std::shared_ptr<Minerva::Vulkan::Renderpass> GetVKRenderpassHandle() const { return m_VKRenderpassHandle; }

		inline void RecreateRenderpass();
		inline void CleanupRenderpass();
	private:
		std::shared_ptr<Minerva::Vulkan::Renderpass> m_VKRenderpassHandle;
	};
}
