#pragma once

#include "../MinervaAPI/Minerva_Device.h"

#include "minerva_vulkan_device.h"

#include <vulkan/vulkan.h>
#include <string>
#include <string_view>
#include <source_location>
#include <vector>
#include <iostream>
#include <memory>

namespace Minerva::Vulkan
{

	

	struct Instance : std::enable_shared_from_this<Instance>
	{
		using ErrorCB = void(std::string_view ErrorString);
		using WarnCB = void(const std::string_view ErrorString);

		struct Setup
		{
			std::string m_sApplicationName{ "My Application" };
			uint32_t m_uApplicationVersion{ 0 };
			bool m_bIsRenderDoc{ false };
			bool m_bIsDebug{ false };
		};

		~Instance();

		void CreateVKInstance(const Setup& _setup, ErrorCB* _errorCB, WarnCB* _warnCB);
		void CreateVKDevice(Minerva::Device& _device, const Minerva::Vulkan::Device::Setup& _setup);
		void SetupDebugMessenger();
		void SetupDebugReport();
		std::shared_ptr<Instance> GetPtr();
		
		//! Required Properties
		VkInstance m_vkInstance {VK_NULL_HANDLE}; //! Actual vulkan instance
		VkDebugUtilsMessengerEXT m_debugMessenger{ nullptr }; //! Vulkan debug messenger
		VkDebugReportCallbackEXT m_debugRptCallback{ nullptr }; //! Vulkan debug report callback
		std::string m_sApplicationName {"My Application"}; //! App name
		std::string m_uApplicationVersion{0}; //! App name
		bool m_bIsDebug {false}; //! Enable debug
		bool m_bIsRenderDoc {false}; //! Enable render doc
		ErrorCB* m_pErrorCallback{ nullptr };
		WarnCB* m_pWarnCallback{ nullptr };

		//! Callbacks for Vulkan side faults ( Takes in Error code )
		void LogError(VkResult errCode, const std::string_view, const std::source_location& location = std::source_location::current());
		void LogWarn(VkResult errCode, const std::string_view, const std::source_location& location = std::source_location::current());
		//! Callbacks for Application side faults ( Error-codeless )
		void LogError(const std::string_view, const std::source_location& location = std::source_location::current());
		void LogWarn(const std::string_view, const std::source_location& location = std::source_location::current());

		std::string_view VKErrorToString(VkResult errorCode);

	private:
		[[nodiscard]] bool CheckExtensionSupport(const std::vector<const char*>& _extensions);
		[[nodiscard]] bool CheckLayerSupport(const std::vector<const char*>& _layers);
		
		//! VK DEBUG CALLBACKS
		static VkBool32 debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		// Debug Report Callback Function
		// Signature copied from Khronos documentation
		static VkBool32 debugReportCallback(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData);
	};

	
}