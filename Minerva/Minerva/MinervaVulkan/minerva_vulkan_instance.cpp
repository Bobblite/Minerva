#include "minerva_vulkan_instance.h"
//#include "../MinervaAPI/Minerva_Exception.h"

#include <iostream>


namespace Minerva::Vulkan
{
	Instance::~Instance()
	{
		// Destroy the Debug Messenger
		auto destroyMessengerFn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyMessengerFn)
			destroyMessengerFn(m_vkInstance, m_debugMessenger, nullptr);

		// Destroy the Instance
		if (m_vkInstance) { vkDestroyInstance(m_vkInstance, nullptr); }
		m_vkInstance = VK_NULL_HANDLE;

		std::cout << "Destroyed Minerva::Vulkan::Instance and Debug Messenger" << std::endl;
	}

	void Instance::CreateVKInstance(const Minerva::Vulkan::Instance::Setup& _setup)
	{
		//todo Create instance with layering based off validation layers.
		std::vector<const char*> Extensions;
		std::vector<const char*> Layers;

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

		if (_setup.m_bIsDebug)
			SetupDebugMessenger();
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
}