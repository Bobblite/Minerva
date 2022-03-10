namespace Minerva
{
	Instance::Instance(const std::string_view _applicationName, uint32_t _applicationVersion, bool _debugEnabled, bool _renderDocEnabled, logCallback* _warnCallback, logCallback* _errorCallback) :
		m_VKInstanceHandle{nullptr}, m_ApplicationName{ _applicationName }, m_ApplicationVersion{ _applicationVersion }, m_DebugEnabled{ _debugEnabled }, m_RenderDocEnabled{ _renderDocEnabled }
	{
			m_VKInstanceHandle = std::make_shared<Minerva::Vulkan::Instance>(_applicationName, _applicationVersion, _debugEnabled, _renderDocEnabled, _warnCallback, _errorCallback);
	}
}
