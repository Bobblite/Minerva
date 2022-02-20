#include "Minerva_Device.h"
#include <iostream>

namespace Minerva
{
	void Device::Create(Minerva::Window& _window, const Minerva::Vulkan::Window::Setup& _setup)
	{
		if (_window.m_pVKWindowHandle != nullptr)
		{
			std::cout << "Window already exist!\n";
			throw std::runtime_error("Window already exist");
		}
	
		_window.m_pVKWindowHandle = std::make_unique<Minerva::Vulkan::Window>();
		_window.m_pVKWindowHandle->Initialize(_setup, m_pVKDeviceHandle);
	}
}
