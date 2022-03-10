#pragma once

namespace Minerva
{
	Instance::Instance(const std::string_view _applicationName, uint32_t _applicationVersion, bool _debugEnabled, bool _renderDocEnabled, logCallback* _warnCallback, logCallback* _errorCallback) :
		m_VKInstanceHandle{ nullptr }
	{
		m_VKInstanceHandle = std::make_shared<Minerva::Vulkan::Instance>(_applicationName, _applicationVersion, _debugEnabled, _renderDocEnabled, _warnCallback, _errorCallback);
	}

	inline std::string_view Instance::GetApplicationName() const { return m_VKInstanceHandle->GetApplicationName(); }
	inline uint32_t Instance::GetApplicationVersion() const { return m_VKInstanceHandle->GetApplicationVersion(); }
	inline bool Instance::IsDebugEnabled() const { return m_VKInstanceHandle->IsDebugEnabled(); }
	inline bool Instance::IsRenderDocEnabled() const { return m_VKInstanceHandle->IsRenderDocEnabled(); }

}
