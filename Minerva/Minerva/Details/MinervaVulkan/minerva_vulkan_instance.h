#pragma once

namespace Minerva::Vulkan
{
	class Instance
	{
	public:
		//using logCallback = void(const std::string_view);

		Instance(const std::string_view _applicationName = "Minerva Application", uint32_t _applicationVersion = 0,
			bool _debugEnabled = false, bool _renderDocEnabled = false, Minerva::Instance::logCallback* _warnCallback = nullptr, Minerva::Instance::logCallback* _errorCallback = nullptr);
		~Instance();

		//! Getters
		inline VkInstance GetVkInstance() const { return m_VKInstance; }
		inline Minerva::Instance::logCallback* GetLogWarnCallback() const { return m_LogWarnCallback; }
		inline Minerva::Instance::logCallback* GetLogErrorCallback() const { return m_LogErrorCallback; }
		inline std::string_view GetApplicationName() const { return m_ApplicationName; }
		inline uint32_t GetApplicationVersion() const { return m_ApplicationVersion; }
		inline bool IsDebugEnabled() const { return m_DebugEnabled; }
		inline bool IsRenderDocEnabled() const { return m_RenderDocEnabled; }

	private:
		// Vulkan properties
		VkInstance m_VKInstance;
		VkDebugReportCallbackEXT m_debugRptCallback{ nullptr };

		// Instance properties
		std::string m_ApplicationName;
		uint32_t m_ApplicationVersion;
		bool m_DebugEnabled;
		bool m_RenderDocEnabled;

		// Callback functions
		Minerva::Instance::logCallback* m_LogWarnCallback;
		Minerva::Instance::logCallback* m_LogErrorCallback;
	};
}

#include "minerva_vulkan_instance.cpp"
