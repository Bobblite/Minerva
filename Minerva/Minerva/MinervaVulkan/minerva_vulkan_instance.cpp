#include "minerva_vulkan_instance.h"
#include <vulkan/vulkan_win32.h> // For extension define
//#include "../MinervaAPI/Minerva_Exception.h"

#include <iostream>
#include <format>
#include <sstream>
#include <algorithm>


namespace Minerva::Vulkan
{
	Instance::~Instance()
	{
		
		// Destroy the Debug Messenger
		auto destroyMessengerFn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyMessengerFn)
			destroyMessengerFn(m_vkInstance, m_debugMessenger, nullptr);

		// Destroy Debug Report Callback
		auto destroyRptCallbackFn = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugReportCallbackEXT");
		if (destroyRptCallbackFn)
			destroyRptCallbackFn(m_vkInstance, m_debugRptCallback, nullptr);

		// Destroy the Instance
		if (m_vkInstance != VK_NULL_HANDLE) { vkDestroyInstance(m_vkInstance, nullptr); }
			m_vkInstance = VK_NULL_HANDLE;
		std::cout << "Destroying Minerva::Vulkan::Instance\n";
		
	}

	void Instance::CreateVKInstance(const Minerva::Vulkan::Instance::Setup& _setup, ErrorCB* _errorCB, WarnCB* _warnCB)
	{
		std::cout << "Creating VKInstance\n";
		//todo Create instance with layering based off validation layers.
		std::vector<const char*> Extensions{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
		std::vector<const char*> Layers;

		m_sApplicationName = _setup.m_sApplicationName;
		m_uApplicationVersion = _setup.m_uApplicationVersion;
		m_bIsDebug = _setup.m_bIsDebug;
		m_bIsRenderDoc = _setup.m_bIsRenderDoc;
		m_pErrorCallback = _errorCB;
		m_pWarnCallback = _warnCB;

		// Application Info
		VkApplicationInfo appInfo {
			.sType =VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = _setup.m_sApplicationName.c_str(),
			.applicationVersion = _setup.m_uApplicationVersion,
			.pEngineName = "Minerva::Vulkan",
			.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
			.apiVersion = VK_API_VERSION_1_2
		};

		// Instance Create Info
		VkInstanceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo
		};

		// Add Layers and Extensions if Debug enabled
		if (_setup.m_bIsDebug)
		{
			Layers.push_back("VK_LAYER_KHRONOS_validation");
			Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			Extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		// Add Layers if RenderDoc enabled
		if (_setup.m_bIsRenderDoc)
			Layers.push_back("VK_LAYER_RENDERDOC_Capture");

		// Check that required Layers are all supported
		if (CheckLayerSupport(Layers))
		{
			createInfo.enabledLayerCount = Layers.size();
			createInfo.ppEnabledLayerNames = Layers.size() ? Layers.data() : nullptr;
		}
		else
			throw std::runtime_error("Layers requested, but not available!");
			//throw MinervaException::MinervaException();

		// Check that required Extensions are all supported
		if (CheckExtensionSupport(Extensions))
		{
			createInfo.enabledExtensionCount = Extensions.size();
			createInfo.ppEnabledExtensionNames = Extensions.size() ? Extensions.data() : nullptr;
		}
		else
			throw std::runtime_error("Extensions requested, but not available!");
		

		// Create Instance
		if (VkResult res{ vkCreateInstance(&createInfo, nullptr, &m_vkInstance) }; res)
		{
			//todo Error handling here
			std::cout << "Failed to create a Vulkan Instance!\n";
		}

		// Setup debug messenger
		if (_setup.m_bIsDebug)
			//SetupDebugMessenger();
			SetupDebugReport();
	}

	void Instance::CreateVKDevice(Minerva::Device& _device, const Minerva::Vulkan::Device::Setup& _setup)
	{
		// Internal function to convert Minerva::Vulkan::Type enum to a Vulkan Queue flag bit
		std::cout << "Creating VKDevice\n";
		static constexpr auto QueueType = []() constexpr
		{
			std::array<VkQueueFlagBits, static_cast<std::size_t>(Minerva::Vulkan::Device::Type::ENUM_COUNT)> QueueType{};

			QueueType[static_cast<std::size_t>(Minerva::Vulkan::Device::Type::RENDER_ONLY)] = VK_QUEUE_GRAPHICS_BIT;
			QueueType[static_cast<std::size_t>(Minerva::Vulkan::Device::Type::RENDER_AND_SWAP)] = VK_QUEUE_GRAPHICS_BIT;
			QueueType[static_cast<std::size_t>(Minerva::Vulkan::Device::Type::COMPUTE)] = VK_QUEUE_COMPUTE_BIT;
			QueueType[static_cast<std::size_t>(Minerva::Vulkan::Device::Type::COPY)] = VK_QUEUE_TRANSFER_BIT;

			return QueueType;
		}();

		// Get all Physical Devices
		uint32_t physicalDeviceCount{ 0 };
		vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCount, nullptr);

		if (physicalDeviceCount == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> PhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCount, PhysicalDevices.data());
		

		//todo This can be extracted to a helper function
		// Physical Device Suitability Check
		// Filter Devices by API version
		for (size_t i{ 0 }; i < PhysicalDevices.size(); ++i)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(PhysicalDevices[i], &deviceProperties);
			if (deviceProperties.apiVersion < VK_API_VERSION_1_2)
			{
				PhysicalDevices.erase(PhysicalDevices.begin() + i);
				i--;
			}
		}
		// Filter out based on discreteness
		// Filter out Discrete GPUs
		if (_setup.m_Discrete == Minerva::Vulkan::Device::Discrete::NON_DISCRETE_ONLY)
		{
			for (size_t i{ 0 }; i < PhysicalDevices.size(); ++i)
			{
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(PhysicalDevices[i], &deviceProperties);

				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					PhysicalDevices.erase(PhysicalDevices.begin() + i);
					i--;
				}
			}
		}
		// Filter out Non-discrete GPUs
		else if (_setup.m_Discrete == Minerva::Vulkan::Device::Discrete::DISCRETE_ONLY)
		{
			for (size_t i{ 0 }; i < PhysicalDevices.size(); ++i)
			{
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(PhysicalDevices[i], &deviceProperties);

				if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					PhysicalDevices.erase(PhysicalDevices.begin() + i);
					i--;
				}
			}
		}
		// If any discrete type, prioritize Discrete GPUs
		else
		{
			std::vector<VkPhysicalDeviceProperties> sortedDeviceProperties(PhysicalDevices.size());
			std::vector<int> sortedDeviceIndex(PhysicalDevices.size());
			
			// Fill device properties vector and index vector
			for (size_t i{ 0 }; i < PhysicalDevices.size(); ++i)
			{
				vkGetPhysicalDeviceProperties(PhysicalDevices[i], sortedDeviceProperties.data()+i);
				sortedDeviceIndex[i] = static_cast<int>(i);
			}

			// Sort based on Discrete type while maintaining stability
			std::sort(sortedDeviceIndex.begin(), sortedDeviceIndex.end(), [&](const int& iA, const int& iB) -> bool
				{
					const int   a = sortedDeviceProperties[iA].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
					const int   b = sortedDeviceProperties[iB].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
					return a < b;
				});
		}

		if (PhysicalDevices.size() == 0)
		{
			LogError("No Physical Devices are suitable for intended API Version and GPU Discreteness");
			return;
		}

		// Check if Device is suitable for intended use based on Queue family
		for (auto& device : PhysicalDevices)
		{
			uint32_t queueFamilyCount{ 0 };
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> QueueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, QueueFamilies.data());

			for (auto& prop : QueueFamilies)
			{
				if (prop.queueFlags & QueueType[static_cast<size_t>(_setup.m_Type)])
				{
					// Create actual device
					std::shared_ptr<Minerva::Vulkan::Device> VulkanDevice{ std::make_shared<Minerva::Vulkan::Device>() };
					uint32_t QueueIndex{ static_cast<uint32_t>(static_cast<size_t>(&prop - QueueFamilies.data())) };
					
					//todo Error control
					VulkanDevice->InitializeDevice(GetPtr(), _setup, QueueIndex, device, std::move(QueueFamilies));
					_device.m_pVKDeviceHandle = VulkanDevice;
					break;
				}
			}
		}
		

	}

	std::shared_ptr<Instance> Instance::GetPtr()
	{
		return shared_from_this();
	}

	void Instance::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		auto createMessengerFn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT");
		if (createMessengerFn(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
			//todo Error control
			std::cerr << "Failed to set up debug messenger!" << std::endl;
	}

	void Instance::SetupDebugReport()
	{
		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
			.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugReportCallback,
			.pUserData = this
		};

		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugReportCallbackEXT");
		
		if (CreateDebugReportCallback)
			if (auto VKErr = CreateDebugReportCallback(m_vkInstance, &dbgCreateInfo, nullptr, &m_debugRptCallback); VKErr)
			{
				LogError(VKErr, "Failed to create the callback that collects all the warnings & errors from vulkan");
			}

		//todo return error handle
	}

	bool Instance::CheckExtensionSupport(const std::vector<const char*>& _extensions)
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

	bool Instance::CheckLayerSupport(const std::vector<const char*>& _layers)
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
				std::cout << focusLayer << " not supported!\n";
				return false;
			}
		}

		return true;
	}


	void Instance::LogError(VkResult errCode, const std::string_view message, const std::source_location& location)
	{
		if (m_pErrorCallback == nullptr) return;

		std::stringstream ss;
		ss << "File: " << location.file_name() << "\n" <<
			"Line: " << location.line() << "\n" <<
			"Column: " << location.column() << "\n" << 
			"Function: " << location.function_name() << "\n" <<
			"VK ERROR (" << errCode << "): " << VKErrorToString(errCode) <<
			"\nMessage: " << message;

		m_pErrorCallback(ss.str().c_str());
	}

	void Instance::LogError(const std::string_view message, const std::source_location& location)
	{
		if (m_pErrorCallback == nullptr) return;
		
		std::stringstream ss;
		ss << "File: " << location.file_name() << "\n" <<
			"Line: " << location.line() << "\n" <<
			"Column: " << location.column() << "\n" <<
			"Function: " << location.function_name() << "\n" <<
			"\nMessage: " << message;

		m_pErrorCallback(ss.str().c_str());
	}

	void Instance::LogWarn(VkResult errCode, const std::string_view message, const std::source_location& location)
	{
		if (m_pWarnCallback == nullptr) return;

		std::stringstream ss;
		ss << "File: " << location.file_name() << "\n" <<
			"Line: " << location.line() << "\n" <<
			"Column: " << location.column() << "\n" <<
			"Function: " << location.function_name() << "\n" <<
			"VK ERROR (" << errCode << "): " << VKErrorToString(errCode) <<
			"\nMessage: " << message;

		m_pWarnCallback(ss.str().c_str());
	}

	void Instance::LogWarn(const std::string_view message, const std::source_location& location)
	{
		if (m_pWarnCallback == nullptr) return;

		std::stringstream ss;
		ss << "File: " << location.file_name() << "\n" <<
			"Line: " << location.line() << "\n" <<
			"Column: " << location.column() << "\n" <<
			"Function: " << location.function_name() << "\n" <<
			"\nMessage: " << message;

		m_pWarnCallback(ss.str().c_str());
	}

	std::string_view Instance::VKErrorToString(VkResult errorCode)
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
	
	VkBool32 Instance::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkBool32 Instance::debugReportCallback(
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

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			std::string temp{ std::format("[{}] Code: {} : {}", pLayerPrefix, messageCode, pMessage) };
			instance.LogError((VkResult)messageCode, temp.c_str());
		}
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			std::string temp{ std::format("[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage) };
			instance.LogWarn((VkResult)messageCode, temp.c_str());
		}
		return VK_FALSE;
	}
}