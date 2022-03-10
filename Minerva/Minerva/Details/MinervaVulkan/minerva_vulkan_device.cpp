namespace Minerva::Vulkan
{
	Device::Device(std::shared_ptr<Minerva::Vulkan::Instance> _instance, Minerva::Device::QueueFamily _queueFamily, Minerva::Device::Type _type) :
		m_VKInstanceHandle{ _instance }, m_VKPhysicalDevice{ VK_NULL_HANDLE }, m_VKDevice{ VK_NULL_HANDLE }, m_VKCommandPool{VK_NULL_HANDLE},
		m_VKMainQueue{ VK_NULL_HANDLE }, m_MainQueueIndex{ 0xffffffff },
		m_QueueFamily{ _queueFamily }, m_Type{ _type }
	{
		if (_instance->GetVkInstance() == VK_NULL_HANDLE)
		{
			Logger::Log_Error("Unable to create Device. Invalid Vulkan Instance.");
			throw std::runtime_error("Unable to create Device. Invalid Vulkan Instance.");
		}


		// Internal function to convert Minerva::Vulkan::Type enum to a Vulkan Queue flag bit
		constexpr auto QueueType = []() constexpr
		{
			std::array<VkQueueFlagBits, static_cast<std::size_t>(Minerva::Device::QueueFamily::ENUM_COUNT)> QueueType{};

			QueueType[static_cast<std::size_t>(Minerva::Device::QueueFamily::RENDER_ONLY)] = VK_QUEUE_GRAPHICS_BIT;
			QueueType[static_cast<std::size_t>(Minerva::Device::QueueFamily::RENDER_AND_SWAP)] = VK_QUEUE_GRAPHICS_BIT;
			QueueType[static_cast<std::size_t>(Minerva::Device::QueueFamily::COMPUTE)] = VK_QUEUE_COMPUTE_BIT;
			QueueType[static_cast<std::size_t>(Minerva::Device::QueueFamily::COPY)] = VK_QUEUE_TRANSFER_BIT;

			return QueueType;
		}();


		// Get all Physical Devices
		uint32_t physicalDeviceCount{ 0 };
		vkEnumeratePhysicalDevices(m_VKInstanceHandle->GetVkInstance(), &physicalDeviceCount, nullptr);

		if (physicalDeviceCount == 0)
		{
			Logger::Log_Error("Failed to find GPUs with Vulkan support!");
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> PhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(m_VKInstanceHandle->GetVkInstance(), &physicalDeviceCount, PhysicalDevices.data());

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
		if (_type == Minerva::Device::Type::NON_DISCRETE_ONLY)
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
		else if (_type == Minerva::Device::Type::DISCRETE_ONLY)
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
				vkGetPhysicalDeviceProperties(PhysicalDevices[i], sortedDeviceProperties.data() + i);
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
			Logger::Log_Error("No Physical Devices are suitable for intended API Version and GPU Discreteness");
			throw std::runtime_error("No Physical Devices are suitable for intended API Version and GPU Discreteness");
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
				// Get the first device that matches
				if (prop.queueFlags & QueueType[static_cast<size_t>(_queueFamily)])
				{
					// Create actual device
					//std::shared_ptr<Minerva::Vulkan::Device> VulkanDevice{ std::make_shared<Minerva::Vulkan::Device>() };

					//uint32_t QueueIndex{ static_cast<uint32_t>(static_cast<size_t>(&prop - QueueFamilies.data())) };
					m_VKPhysicalDevice = device;
					m_MainQueueIndex = static_cast<uint32_t>(static_cast<size_t>(&prop - QueueFamilies.data()));


					// Create Vulkan (logical) Device based on Queue Family
					switch (_queueFamily)
					{
						using enum Minerva::Device::QueueFamily;
					case COMPUTE:
					case COPY:
						Logger::Log_Error("Failed to create Vulkan Device. Queue Family unsupported.");
						throw std::runtime_error("Failed to create Vulkan Device. Queue Family unsupported.");
					case RENDER_AND_SWAP:
					case RENDER_ONLY:
						CreateGraphicsDevice(QueueFamilies);
					default:
						break;
					}

					// Get all required device queues
					vkGetDeviceQueue(m_VKDevice, m_MainQueueIndex, 0, &m_VKMainQueue);
				}
			}
		}

		// Create Command Pool for Transfers
		// TODO CURRENTLY USES MAIN QUEUE FOR TRANSFERS
		VkCommandPoolCreateInfo commandPoolCreateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = m_MainQueueIndex
		};

		if (auto VkErr{ vkCreateCommandPool(m_VKDevice, &commandPoolCreateInfo, nullptr, &m_VKCommandPool) }; VkErr)
		{
			Logger::Log_Error("Unable to create Command Pool. vkCreateCommandPool failed.");
			throw std::runtime_error("Unable to create Command Pool. vkCreateCommandPool failed.");
		}
	}

	Device::~Device()
	{
		if (m_VKCommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_VKDevice, m_VKCommandPool, nullptr);

		if (m_VKDevice != VK_NULL_HANDLE)
		{
			vkDestroyDevice(m_VKDevice, nullptr);
			m_VKDevice = VK_NULL_HANDLE;
		}
	}

	void Device::CreateGraphicsDevice(const std::vector<VkQueueFamilyProperties>& _deviceProperties)
	{
		// Queue Create Info (One for each queue type)
		static const std::array queuePriorities = { 0.f };

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo{
			VkDeviceQueueCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_MainQueueIndex,
			.queueCount = static_cast<uint32_t>(queuePriorities.size()),
			.pQueuePriorities = queuePriorities.data()
		}
		};

		// Creating Device
		VkPhysicalDeviceFeatures DeviceFeatures{};
		vkGetPhysicalDeviceFeatures(m_VKPhysicalDevice, &DeviceFeatures);

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
		std::vector<const char*> ValidationLayers{ "VK_LAYER_KHRONOS_validation" };

		if (m_VKInstanceHandle->IsDebugEnabled())
		{
			DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			DeviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
		}

		if (auto VKErr = vkCreateDevice(m_VKPhysicalDevice, &DeviceCreateInfo, nullptr, &m_VKDevice); VKErr)
		{
			Logger::Log_Error(VKErr, "Failed to create Vulkan Graphical Device");
			throw std::runtime_error("Failed to create Logical Device (Graphics)");
		}
	}

	void Device::CopyBuffer(VkBuffer _src, VkBuffer _dst, VkDeviceSize _size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_VKCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_VKDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// Begin recording
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = _size;
		vkCmdCopyBuffer(commandBuffer, _src, _dst, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		// Submit operation
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_VKMainQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_VKMainQueue);

		vkFreeCommandBuffers(m_VKDevice, m_VKCommandPool, 1, &commandBuffer);
	}
}
