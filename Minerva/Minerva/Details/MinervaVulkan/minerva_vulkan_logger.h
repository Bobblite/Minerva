#pragma once

namespace Minerva::Vulkan
{
	class Logger
	{
	public:
		using logCallback = void(const std::string_view);

		//! With VKResult code
		static void Log_Warn(const VkResult _errCode, const std::string_view _message, logCallback* _warnCB = nullptr, const std::source_location& _location = std::source_location::current());
		static void Log_Error(const VkResult _errCode, const std::string_view _message, logCallback* _errorCB = nullptr, const std::source_location& _location = std::source_location::current());

		//! Without VKResult code
		static void Log_Warn(const std::string_view _message, logCallback* _warnCB = nullptr, const std::source_location& _location = std::source_location::current());
		static void Log_Error(const std::string_view _message, logCallback* _errorCB = nullptr, const std::source_location& _location = std::source_location::current());
	};
}

#include "minerva_vulkan_logger.cpp"