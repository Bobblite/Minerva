#pragma once

#include "../MinervaVulkan/minerva_vulkan_instance.h"

#include <string>
#include <memory>

namespace Minerva
{
	struct Instance
	{

		~Instance() noexcept;
		std::unique_ptr<Minerva::Vulkan::Instance> m_pVKInstanceHandle{ nullptr };
		std::string m_sApplicationName{ "My Application" };
		uint32_t m_uApplicationVersion{ 0 };

	};

	void CreateInstance(Instance& _instance, const Minerva::Vulkan::Instance::Setup& _setup);
}