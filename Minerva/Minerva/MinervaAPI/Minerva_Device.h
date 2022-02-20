#pragma once

#include "Minerva_Window.h"
#include "../MinervaVulkan/minerva_vulkan_device.h"

#include <memory>

namespace Minerva
{
	struct Device
	{
		std::shared_ptr<Minerva::Vulkan::Device> m_pVKDeviceHandle{nullptr};

		void Create(Minerva::Window& _window, const Minerva::Vulkan::Window::Setup& _setup);
		//todo Create functions for Window, input, etc here
	};
}
