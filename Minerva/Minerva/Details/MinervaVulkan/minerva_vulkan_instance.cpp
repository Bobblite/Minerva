bool CheckLayerSupport(const std::vector<const char*>& _layers);
bool CheckExtensionSupport(const std::vector<const char*>& _extensions);


namespace Minerva::Vulkan
{

	static VkBool32 debugReportCallback(
		VkDebugReportFlagsEXT                       flags,
		VkDebugReportObjectTypeEXT                  objectType,
		uint64_t                                    object,
		size_t                                      location,
		int32_t                                     messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		auto& instance = *reinterpret_cast<Instance*>(pUserData);

		std::ostringstream message;
		message << "[" << pLayerPrefix << "]" << "Code: " << messageCode << " : " << pMessage;

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			//std::string temp{ std::format("[{}] Code: {} : {}", pLayerPrefix, messageCode, pMessage) };
			Logger::Log_Error((VkResult)messageCode, message.str().c_str(), instance.GetLogErrorCallback());
		}
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			//std::string temp{ std::format("[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage) };
			Logger::Log_Warn((VkResult)messageCode, message.str().c_str(), instance.GetLogWarnCallback());
		}
		return VK_FALSE;
	}

	Instance::Instance(const std::string_view _applicationName, uint32_t _applicationVersion,
		bool _debugEnabled, bool _renderDocEnabled, Minerva::Instance::logCallback* _warnCallback, Minerva::Instance::logCallback* _errorCallback) :
		m_VKInstance{VK_NULL_HANDLE}, m_ApplicationName{ _applicationName }, m_ApplicationVersion{ _applicationVersion },
		m_DebugEnabled{ _debugEnabled }, m_RenderDocEnabled{ _renderDocEnabled }, m_LogWarnCallback{ _warnCallback }, m_LogErrorCallback{ _errorCallback }
	{
		// Layers and Extension Vectors
		std::vector<const char*> Extensions{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		std::vector<const char*> Layers;

		// Application Info
		VkApplicationInfo appInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = m_ApplicationName.c_str(),
			.applicationVersion = m_ApplicationVersion,
			.pEngineName = "Minerva::Vulkan",
			.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
			.apiVersion = VK_API_VERSION_1_2
		};

		// Create Info
		VkInstanceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo
		};

		// Add Layers and Extensions if Debug enabled
		if (m_DebugEnabled)
		{
			Layers.push_back("VK_LAYER_KHRONOS_validation");
			Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			Extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		// Add Layers if RenderDoc enabled
		if (m_RenderDocEnabled)
			Layers.push_back("VK_LAYER_RENDERDOC_Capture");

		// Check that required Layers are all supported
		if (CheckLayerSupport(Layers))
		{
			createInfo.enabledLayerCount = Layers.size();
			createInfo.ppEnabledLayerNames = Layers.size() ? Layers.data() : nullptr;
		}
		else
		{
			Logger::Log_Error("Layers requested, but not available!");
			throw std::runtime_error("Layers requested, but not available!");
		}

		// Check that required Extensions are all supported
		if (CheckExtensionSupport(Extensions))
		{
			createInfo.enabledExtensionCount = Extensions.size();
			createInfo.ppEnabledExtensionNames = Extensions.size() ? Extensions.data() : nullptr;
		}
		else
		{
			Logger::Log_Error("Extensions requested, but not available!");
			throw std::runtime_error("Extensions requested, but not available!");
		}

		// Create Instance
		if (VkResult res{ vkCreateInstance(&createInfo, nullptr, &m_VKInstance) }; res)
		{
			//todo Error handling here
			std::cout << "Failed to create a Vulkan Instance!\n";
		}

		// Setup Debug Callback
		if (m_DebugEnabled)
		{
			VkDebugReportCallbackCreateInfoEXT dbgCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
				.pNext = nullptr,
				.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
				.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugReportCallback,
				.pUserData = this
			};

			PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VKInstance, "vkCreateDebugReportCallbackEXT");

			if (CreateDebugReportCallback)
				if (auto VKErr = CreateDebugReportCallback(m_VKInstance, &dbgCreateInfo, nullptr, &m_debugRptCallback); VKErr)
				{
					Logger::Log_Error(VKErr, "Failed to create the callback that collects all the warnings & errors from vulkan");
				}
		}
	}

	Instance::~Instance()
	{
		// Destroy Debug Report Callback
		auto destroyRptCallbackFn = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VKInstance, "vkDestroyDebugReportCallbackEXT");
		if (destroyRptCallbackFn)
			destroyRptCallbackFn(m_VKInstance, m_debugRptCallback, nullptr);

		// Destroy the Instance
		if (m_VKInstance != VK_NULL_HANDLE) { vkDestroyInstance(m_VKInstance, nullptr); }
		m_VKInstance = VK_NULL_HANDLE;
	}

}

bool CheckLayerSupport(const std::vector<const char*>& _layers)
{
	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> supportedLayerList(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, supportedLayerList.data());

	for (const char* focusLayer : _layers)
	{
		bool found{ false };

		for (const auto& supportedLayer : supportedLayerList)
		{
			if (strcmp(focusLayer, supportedLayer.layerName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			std::ostringstream oss;
			oss << focusLayer << " not supported!\n";
			Minerva::Vulkan::Logger::Log_Error(oss.str());
			return false;
		}
	}

	return true;
}

bool CheckExtensionSupport(const std::vector<const char*>& _extensions)
{
	uint32_t extensionCount{ 0 };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> supportedExtensionsList(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensionsList.data());
	for (const char* focusExtension : _extensions)
	{
		bool found{ false };

		for (const auto& supportedExtension : supportedExtensionsList)
		{
			if (strcmp(focusExtension, supportedExtension.extensionName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			std::cout << focusExtension << " not supported!\n";
			return false;
		}
	}

	return true;
}

