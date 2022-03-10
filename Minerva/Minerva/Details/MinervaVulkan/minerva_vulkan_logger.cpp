std::string_view VKErrorToString(VkResult errorCode);

namespace Minerva::Vulkan
{
	void Logger::Log_Warn(const VkResult _errCode, const std::string_view _message, logCallback* _warnCB, const std::source_location& _location)
	{
		std::ostringstream formatMessage;
		formatMessage << "[WARN] File: " << _location.file_name() << "\n" <<
			"Line: " << _location.line() << "\n" <<
			"Column: " << _location.column() << "\n" <<
			"Function: " << _location.function_name() << "\n" <<
			"VK ERROR (" << _errCode << "): " << VKErrorToString(_errCode) <<
			"\nMessage: " << _message;

		if (_warnCB)
			_warnCB(formatMessage.str().c_str());
		else
			std::cout << formatMessage.str() << std::endl;
	}

	void Logger::Log_Error(const VkResult _errCode, const std::string_view _message, logCallback* _errorCB, const std::source_location& _location)
	{
		std::cout << "test\n";
		std::ostringstream formatMessage;
		formatMessage << "[ERROR] File: " << _location.file_name() << "\n" <<
			"Line: " << _location.line() << "\n" <<
			"Column: " << _location.column() << "\n" <<
			"Function: " << _location.function_name() << "\n" <<
			"VK ERROR (" << _errCode << "): " << VKErrorToString(_errCode) <<
			"\nMessage: " << _message;

		if (_errorCB)
			_errorCB(formatMessage.str().c_str());
		else
			std::cout << formatMessage.str() << std::endl;
	}

	void Logger::Log_Warn(const std::string_view _message, logCallback* _warnCB, const std::source_location& _location)
	{
		std::ostringstream formatMessage;
		formatMessage << "[WARN] File: " << _location.file_name() << "\n" <<
			"Line: " << _location.line() << "\n" <<
			"Column: " << _location.column() << "\n" <<
			"Function: " << _location.function_name() << "\n" <<
			"\nMessage: " << _message;

		if (_warnCB)
			_warnCB(formatMessage.str().c_str());
		else
			std::cout << formatMessage.str() << std::endl;
	}

	void Logger::Log_Error(const std::string_view _message, logCallback* _errorCB, const std::source_location& _location)
	{
		std::ostringstream formatMessage;
		formatMessage << "[ERROR] File: " << _location.file_name() << "\n" <<
			"Line: " << _location.line() << "\n" <<
			"Column: " << _location.column() << "\n" <<
			"Function: " << _location.function_name() << "\n" <<
			"\nMessage: " << _message;

		if (_errorCB)
			_errorCB(formatMessage.str().c_str());
		else
			std::cout << formatMessage.str() << std::endl;
	}
}



std::string_view VKErrorToString(VkResult errorCode)
{
	switch (errorCode)
	{
#define STR(r) case VK_ ##r: return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);

#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}
