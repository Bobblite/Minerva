#pragma once

namespace Minerva::Vulkan
{
	class Renderpass
	{
	public:
		Renderpass(std::shared_ptr<Minerva::Vulkan::Device> _device, std::shared_ptr<Minerva::Vulkan::Window> _window, float* _clearColor);
		~Renderpass();

		inline VkRenderPass GetVKRenderPass() const { return m_VKRenderPass; }
		inline VkClearValue GetVkClearValue() const { return m_VKClearValue; }
		inline std::vector<VkFramebuffer>& GetVKFramebuffers() { return m_VKFramebuffers; }

		void CleanupRenderpass();
		void RecreateRenderpass();
	private:
		// Private interface handles
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;
		std::shared_ptr<Minerva::Vulkan::Window> m_VKWindowHandle;


		// Vulkan properties
		VkRenderPass m_VKRenderPass;
		std::vector<VkFramebuffer> m_VKFramebuffers;
		VkClearValue m_VKClearValue;
		
	};
}

#include "minerva_vulkan_renderpass.cpp"