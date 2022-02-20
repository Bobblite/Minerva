#pragma once

#include "../MinervaVulkan/minerva_vulkan_instance.h"

#include <string>
#include <memory>

namespace Minerva
{
	struct Instance
	{

		~Instance() noexcept;

		std::shared_ptr<Minerva::Vulkan::Instance> m_pVKInstanceHandle{ nullptr };
		std::string m_sApplicationName{ "My Application" };
		uint32_t m_uApplicationVersion{ 0 };

		void Create(Minerva::Device& _device, const Minerva::Vulkan::Device::Setup& _setup)
		{
			if (!m_pVKInstanceHandle)
				throw std::runtime_error("Unable to create device, NULL instance handle");

			m_pVKInstanceHandle->CreateVKDevice(_device, _setup);
		}
	};

	void CreateInstance(Instance& _instance, const Minerva::Vulkan::Instance::Setup& _setup, Minerva::Vulkan::Instance::ErrorCB* _errorCB = nullptr, Minerva::Vulkan::Instance::WarnCB* _warnCB = nullptr);
}