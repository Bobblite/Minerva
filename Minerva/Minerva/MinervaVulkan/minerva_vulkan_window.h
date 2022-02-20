#pragma once

#include <vulkan/vulkan.h>
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include <array>

namespace Minerva::Vulkan
{
    struct Instance;
    struct Device;

	struct Window
	{
        struct Setup
        {
            int                 m_Width{ 1280 };
            int                 m_Height{ 720 };
            bool                m_bFullScreen{ false };
            bool                m_bClearOnRender{ true };
            bool                m_bVSyncOn{ false };
            float               m_ClearColorR{ 0.45f };
            float               m_ClearColorG{ 0.45f };
            float               m_ClearColorB{ 0.45f };
            float               m_ClearColorA{ 1.0f };
        };

        ~Window();
        void Initialize(const Minerva::Vulkan::Window::Setup& _setup, std::shared_ptr<Device> _pDevice);
        void SetWindowValues(int _width, int _height, bool _isMinimized);
        void SetMinimized();
        bool ProcessInput();
        void DrawFrame();
        void RecreateSwapChain();
        void CleanupSwapChain();

        const int MAX_FRAMES_IN_FLIGHT = 2;

        //! SURFACE AND SWAPCHAIN PROPERTIES
        std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle{ nullptr };
        std::shared_ptr<Minerva::Vulkan::Instance> m_VKInstanceHandle{ nullptr };
        std::array<VkClearValue, 2> m_VKClearValue{ VkClearValue{.color = {.float32 = {0,0,0,1}} }, VkClearValue{.depthStencil{ 1.0f, 0 } } };
        bool m_bClearOnRender{ true };
        int m_Width{ 1280 };
        int m_Height{ 720 };
        bool m_bResized{ false };
        bool m_bMinimized{ false };
        bool m_bFullscreen{ false };
        bool m_bVSync{ false };
        HWND m_hWND{};
        HMODULE m_hInstance;
        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
        VkSurfaceFormatKHR m_SurfaceFormat{};
        VkSurfaceCapabilitiesKHR m_SurfaceCapabilities{};
        VkPresentModeKHR m_PresentMode{};
        VkExtent2D m_swapExtent{};
        VkFormat m_swapChainImageFormat{};
        VkSwapchainKHR m_SwapChain{};
        std::vector<VkImage> m_SwapChainImages;
        uint32_t m_ImageCount{ 0 };
        std::vector<VkImageView> m_SwapChainImageView{};

        //! PIPELINE PROPERTIES
        VkPipeline m_Pipeline{};
        VkPipelineLayout m_PipelineLayout{};
        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
        VkRenderPass m_RenderPass{};

        //! DRAWING PROPERTIES
        std::vector<VkFramebuffer> m_SwapChainFrameBuffers;
        VkCommandPool m_CommandPool;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame{ 0 };

    private:
        //! SURFACE AND SWAPCHAIN SPECIFIC METHODS
        int CreateSurface();
        int CreateWindowClass(HINSTANCE hinstance, WNDPROC wndproc);
        int CreateSystemWindow(HINSTANCE   hInstance, HWND& hWnd, bool bFullScreen, int width, int height);
        void CheckPresentationSupport();
        void ChooseSwapExtent();
        void CreateSwapChain();
        void CreateImageViews();
        void CheckAndConfigureSwapChainSupport();

        //! PIPELINE SPECIFIC METHODS
        void CreateRenderPass();
        void CreateGraphicsPipeline();
        VkShaderModule CreateShaderModule(const std::vector<char>& code);

        //! DRAWING SPECIFIC METHODS
        void CreateFrameBuffers();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSyncObjects();
        
        static std::vector<char> LoadShader(const std::string& _filePath);
        

    };
}
