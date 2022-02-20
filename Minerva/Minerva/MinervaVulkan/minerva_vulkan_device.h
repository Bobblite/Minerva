#pragma once

#include "minerva_vulkan_window.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Minerva
{
    struct Window;
}

namespace Minerva::Vulkan
{
    struct Instance;

	struct Device
	{
        enum class Type : std::uint8_t
        {
            RENDER_AND_SWAP
            , RENDER_ONLY
            , COMPUTE
            , COPY
            , ENUM_COUNT
        };

        enum class Discrete : std::uint8_t
        {
            ANY_GPU
            , DISCRETE_ONLY
            , NON_DISCRETE_ONLY
        };

		struct Setup
		{
            Type m_Type{ Type::RENDER_AND_SWAP };
            Discrete m_Discrete{ Discrete::ANY_GPU };
		};

        ~Device();
        void InitializeDevice(std::shared_ptr<Minerva::Vulkan::Instance> _instance, const Minerva::Vulkan::Device::Setup& _setup, uint32_t _mainQueueIndex, VkPhysicalDevice _physicalDevice, std::vector<VkQueueFamilyProperties> _properties);

        void Create(Minerva::Window& _window, const Minerva::Vulkan::Window::Setup& _setup, std::shared_ptr<Minerva::Vulkan::Device> _pDevice);

		std::shared_ptr<Minerva::Vulkan::Instance> m_Instance{ nullptr };
		VkPhysicalDevice m_VKPhysicalDevice{ VK_NULL_HANDLE };
		VkDevice m_VKDevice{ VK_NULL_HANDLE };
        std::uint32_t m_uMainQueueIndex{ 0xffffffff };
        //std::uint32_t m_uTransferQueueIndex{ 0xffffffff };
        VkQueue m_VKMainQueue{ VK_NULL_HANDLE };
        //VkQueue m_VKTransferQueue{ VK_NULL_HANDLE };

    private:
        static void CreateGraphicsDevice(Minerva::Vulkan::Device& device, const bool enableValidation, const std::vector<VkQueueFamilyProperties>& _deviceProperties);
	};
}
