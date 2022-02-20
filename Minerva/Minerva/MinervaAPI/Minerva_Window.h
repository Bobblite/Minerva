#pragma once

#include "../MinervaVulkan/minerva_vulkan_window.h"

#include <memory>

namespace Minerva
{
	struct Window
	{
		std::unique_ptr<Minerva::Vulkan::Window> m_pVKWindowHandle{ nullptr };

		bool ProcessInput();
		void DrawFrame();
	};
}
