#pragma once

namespace Minerva
{
	class Instance
	{
	public:
		using logCallback = void(const std::string_view);

		Instance(const std::string_view _applicationName = "Minerva Application",
			uint32_t _applicationVersion = 0, bool _debugEnabled = false, bool _renderDocEnabled = false,
			logCallback* _warnCallback = nullptr, logCallback* _errorCallback = nullptr);
		
		inline std::shared_ptr<Minerva::Vulkan::Instance> GetVKInstanceHandle() const { return m_VKInstanceHandle; }
		inline std::string_view GetApplicationName() const;
		inline uint32_t GetApplicationVersion() const;
		inline bool IsDebugEnabled() const;
		inline bool IsRenderDocEnabled() const;

	private:
		// Private interface handle
		std::shared_ptr<Minerva::Vulkan::Instance> m_VKInstanceHandle;
	};
}
