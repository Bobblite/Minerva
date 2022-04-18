#pragma once

namespace Minerva::Vulkan
{
	class Device
	{
	public:
		Device(std::shared_ptr<Minerva::Vulkan::Instance> _instance, Minerva::Device::QueueFamily _queueFamily, Minerva::Device::Type _type);
		~Device();

		void CopyBuffer(VkBuffer _src, VkBuffer _dst, VkDeviceSize _size);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer _cmdBuffer);

		inline std::shared_ptr<Minerva::Vulkan::Instance> GetVKInstanceHandle() const { return m_VKInstanceHandle; }
		inline VkPhysicalDevice GetVKPhysicalDevice() const { return m_VKPhysicalDevice; }
		inline VkDevice GetVKDevice() const { return m_VKDevice; }
		inline VkDescriptorPool GetVKDescriptorPool() const { return m_VKDescriptorPool; }
		inline VkQueue GetMainQueue() const { return m_VKMainQueue; }
		inline uint32_t GetMainQueueIndex() const { return m_MainQueueIndex; }
		inline Minerva::Device::QueueFamily GetQueueFamily() const { return m_QueueFamily; }
		inline Minerva::Device::Type GetDeviceType() const { return m_Type; }
	private:
		// Minerva::Vulkan Object handles
		std::shared_ptr<Minerva::Vulkan::Instance> m_VKInstanceHandle;

		// Vulkan properties
		VkPhysicalDevice m_VKPhysicalDevice;
		VkDevice m_VKDevice;
		VkCommandPool m_VKCommandPool;
		VkDescriptorPool m_VKDescriptorPool;
		std::array<VkDescriptorPoolSize, 2> m_VKDescriptorPoolSizes;

		// Queue properties
		VkQueue m_VKMainQueue;
		uint32_t m_MainQueueIndex;

		// Minerva properties
		Minerva::Device::QueueFamily m_QueueFamily;
		Minerva::Device::Type m_Type;

		// Helper function to create graphics device
		void CreateGraphicsDevice(const std::vector<VkQueueFamilyProperties>& _deviceProperties);
	};
}

#include "minerva_vulkan_device.cpp"
