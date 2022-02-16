#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <iostream>

namespace Minerva::Vulkan
{
	struct Instance
	{
		struct Setup
		{
			std::string m_sApplicationName{ "My Application" };
			uint32_t m_uApplicationVersion{ 0 };
			bool m_bIsRenderDoc{ false };
			bool m_bIsDebug{ false };
		};

		~Instance();

		void CreateVKInstance(const Setup& _setup);
		void SetupDebugMessenger();
		
		VkInstance m_vkInstance {VK_NULL_HANDLE};
		VkDebugUtilsMessengerEXT m_debugMessenger{ nullptr };
		std::string m_sApplicationName {"My Application"};
		bool m_bIsDebug {false};
		bool m_bIsRenderDoc {false};

	private:
		[[nodiscard]] bool CheckExtensionSupport(const std::vector<const char*>& _extensions);
		[[nodiscard]] bool CheckLayerSupport(const std::vector<const char*>& _layers);
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}
	};

	
}