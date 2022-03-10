#pragma once

namespace Minerva::Vulkan
{
	class Window
	{
	public:
		Window(std::shared_ptr<Minerva::Vulkan::Device> _device, bool _fullscreen = false, bool _vsync = true, int _width = 1920, int _height = 1080);
		~Window();

		// Get/Set
		inline std::shared_ptr<Minerva::Vulkan::Instance> GetInstanceHandle() const { return m_VKInstanceHandle; }
		inline std::shared_ptr<Minerva::Vulkan::Device> GetDeviceHandle() const { return m_VKDeviceHandle; }
		inline VkSurfaceKHR GetVKSurface() const { return m_VKSurface; }

		inline void SetFullScreen(bool _isFullscreen) { m_FullScreen = _isFullscreen; }
		inline void SetVSync(bool _isVSync) { m_VSync = _isVSync; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline void SetWidth(int _width) { m_Width = _width; }
		inline void SetHeight(int _height) { m_Height = _height; }
		inline void SetMinimized(bool _minimized) { m_Minimized = _minimized; }

		inline const VkExtent2D GetVKSwapExtent() const { return m_VKSwapExtent; }
		inline const VkFormat GetVKImageFormat() const { return m_VKSwapChainImageFormat; }
		inline const std::vector<VkImageView>& GetVKSwapImageViews() const { return m_VKSwapChainImageViews; }

		// Actual Functionalities
		Minerva::CommandBuffer GetCommandBuffer();
		Minerva::Window::RenderStatus BeginRender(std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass);
		Minerva::Window::RenderStatus PageFlip();

		void SetWindowValues(int _width, int _height, bool _isMinimized);
		void CleanupSwapchain();
		void RecreateSwapchain();

	private:
		// Minerva::Vulkan Object handles
		std::shared_ptr<Minerva::Vulkan::Instance> m_VKInstanceHandle;
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;
		std::shared_ptr<Minerva::Vulkan::Renderpass> m_VKRenderpassHandle;


		// Vulkan properties
		VkSurfaceKHR m_VKSurface;
		VkSwapchainKHR m_VKSwapChain;
		VkSurfaceCapabilitiesKHR m_VKSurfaceCapabilities;
		VkSurfaceFormatKHR m_VKSurfaceFormat;
		VkPresentModeKHR m_VKPresentMode;
		std::vector<VkImage> m_VKSwapChainImages;
		std::vector<VkImageView> m_VKSwapChainImageViews;
		VkFormat m_VKSwapChainImageFormat;
		VkExtent2D m_VKSwapExtent;
		VkCommandPool m_VKCommandPool;
		std::vector<VkCommandBuffer> m_VKCommandBuffers;
		std::vector<CommandBuffer> m_CommandBuffers;

		// Synchronization
		const int MAX_FRAMES_IN_FLIGHT = 2;
		std::vector<VkSemaphore> m_VKImageAvailableSemaphores; // Signals that image in a frame has been acquired from swap chain
		std::vector<VkSemaphore> m_VKRenderCompleteSemaphores; // Signals that rendering in a frame is finished
		std::vector<VkFence> m_VKInFlightFences; // Ensures a frame is rendering at one time
		uint32_t m_CurrentFrame;
		uint32_t m_ImageIndex;

		// WIN32 API properties
		HMODULE m_hInstance;
		HWND m_hWND;

		// Window Properties
		int m_Width;
		int m_Height;
		bool m_FullScreen;
		bool m_VSync;
		bool m_Resized;
		bool m_Minimized;

		// Helper functions
		void CreateSurface();
		void CheckPresentationSupport();
		void CheckAndConfigureSwapChainSupport();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateCommandBuffers();
		void CreateSyncObjects();
	};
}

#include "minerva_vulkan_window.cpp"