#include "minerva_vulkan_device.h"
#include "minerva_vulkan_instance.h"

#include <array>

namespace Minerva::Vulkan
{
	Device::~Device()
	{
		if (m_VKDevice != VK_NULL_HANDLE)
		{
			vkDestroyDevice(m_VKDevice, nullptr);
			m_VKDevice = VK_NULL_HANDLE;
			std::cout << "Destroying Minerva::Vulkan::Device\n";
		}
		
	}

	void Device::CreateGraphicsDevice(Minerva::Vulkan::Device& device, const bool enableValidation, const std::vector<VkQueueFamilyProperties>& _deviceProperties)
	{
		std::cout << "Creating Graphics Device\n";
		//todo Transfer queue stuff
		
		// Queue Create Info (One for each queue type)
		static const std::array queuePriorities = { 0.f };

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo{
			VkDeviceQueueCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = device.m_uMainQueueIndex,
			.queueCount = static_cast<uint32_t>(queuePriorities.size()),
			.pQueuePriorities = queuePriorities.data()
		}
		};

		

		// Creating Device
		VkPhysicalDeviceFeatures DeviceFeatures{};
		vkGetPhysicalDeviceFeatures(device.m_VKPhysicalDevice, &DeviceFeatures);
		
		//todo Left as empty for now
		// deviceFeatures.shaderClipDistance = true;
		// deviceFeatures.shaderCullDistance = true;
		// deviceFeatures.samplerAnisotropy = true;

		// Required Extensions
		static std::vector<const char*> EnabledDeviceExtensions;
		EnabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		//EnabledDeviceExtensions.push_back(VK_NV_GLSL_SHADER_EXTENSION_NAME); // nVidia useful extension to be able to load GLSL shaders

		// CreateDeviceInfo
		VkDeviceCreateInfo DeviceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = nullptr,
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size()),
			.pQueueCreateInfos = queueCreateInfo.data(),
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = static_cast<uint32_t>(EnabledDeviceExtensions.size()),
			.ppEnabledExtensionNames = EnabledDeviceExtensions.data(),
			.pEnabledFeatures = &DeviceFeatures
		};

		//! <VALIDATION LAYERS IN DEVICE ARE NOT DEPRECATED. THESE WILL NOT BE CHECKED BY UP-TO-DATE IMPLEMENTATIONS OF VULKAN>
		//! Set validation layers for backwards compatibility
		std::vector<const char*> ValidationLayers{"VK_LAYER_KHRONOS_validation"};

		if (device.m_Instance->m_bIsDebug)
		{
			DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			DeviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
		}

		if (auto VKErr = vkCreateDevice(device.m_VKPhysicalDevice, &DeviceCreateInfo, nullptr, &device.m_VKDevice); VKErr)
		{
			device.m_Instance->LogError(VKErr, "Failed to create Vulkan Graphical Device");
			throw std::runtime_error("Failed to create Logical Device (Graphics)");
		}
	}

	void Device::InitializeDevice(std::shared_ptr<Minerva::Vulkan::Instance> _instance, const Minerva::Vulkan::Device::Setup& _setup, uint32_t _mainQueueIndex, VkPhysicalDevice _physicalDevice, std::vector<VkQueueFamilyProperties> _properties)
	{
		std::cout << "Initializing VKDevice\n";
		m_Instance = _instance;
		m_VKPhysicalDevice = _physicalDevice;
		m_uMainQueueIndex = _mainQueueIndex;

		// Create Vulkan (logical) Device based on Queue Family
		switch (_setup.m_Type)
		{
			using enum Type;
		case COMPUTE:
		case COPY:
			m_Instance->LogError("Failed to create Vulkan Device. Queue Family unsupported.");
			return;
		case RENDER_AND_SWAP:
		case RENDER_ONLY:
			CreateGraphicsDevice(*this, m_Instance->m_bIsDebug, _properties);
		default:
			break;
		}

		// Get all required device queues
		vkGetDeviceQueue(m_VKDevice, m_uMainQueueIndex, 0, &m_VKMainQueue);
	}

	void Device::Create(Minerva::Window& _window, const Minerva::Vulkan::Window::Setup& _setup, std::shared_ptr<Minerva::Vulkan::Device> _pDevice)
	{
		auto NewWindow{ std::make_unique<Minerva::Vulkan::Window>() };
		NewWindow->Initialize(_setup, _pDevice);
		_window.m_pVKWindowHandle = std::move(NewWindow);
	}
}

