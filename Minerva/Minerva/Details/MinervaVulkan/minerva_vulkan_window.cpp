#include "minerva_vulkan_window.h"
namespace Minerva::Vulkan
{
	void CreateWindowClass(HINSTANCE _hInstance, WNDPROC _wndproc);
    void CreateSystemWindow(HINSTANCE  _hInstance, HWND& _hWnd, bool _bFullScreen, int _width, int _height);
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    

    Window::Window(std::shared_ptr<Minerva::Vulkan::Device> _device, bool _fullscreen, bool _vsync, int _width, int _height) :
        m_VKInstanceHandle{ _device->GetVKInstanceHandle() }, m_VKDeviceHandle{ _device }, m_VKRenderpassHandle{ nullptr }, // Handles
        m_VKSurface{ VK_NULL_HANDLE }, m_VKSwapChain{ VK_NULL_HANDLE }, // Vulkan properties
        m_VKSwapChainImages{}, m_VKSwapChainImageViews{}, m_VKSwapChainImageFormat{}, m_VKSwapExtent{ 0 },
        m_VKCommandPool{ VK_NULL_HANDLE }, m_VKCommandBuffers{}, // Command pool and buffers
        m_VKImageAvailableSemaphores{}, m_VKRenderCompleteSemaphores{}, m_VKInFlightFences{}, m_CurrentFrame{ 0 }, m_ImageIndex{ 0 }, // Sync objects
        m_hInstance{ nullptr }, m_hWND{ nullptr }, // Win32 properties
        m_Width{ _width }, m_Height{ _height }, m_FullScreen{ _fullscreen }, m_VSync{ _vsync }, // Window properties
        m_Minimized{ false }, m_Resized{ false }
	{
		if (!m_VKInstanceHandle || !m_VKDeviceHandle)
		{
			Logger::Log_Error("Unable to create a Window. Instance or Device is NULL");
			throw std::runtime_error("Unable to create a Window. Instance or Device is NULL");
		}

        // Create Win32 Window
        CreateWindowClass(m_hInstance, WndProc);
        CreateSystemWindow(m_hInstance, m_hWND, m_FullScreen, m_Width, m_Height);
        if (m_hWND) SetWindowLongPtr(m_hWND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        // Create VKSurface
        CreateSurface(); // Creates the VKSurface
        CheckPresentationSupport(); // Check if the physical device supports presentation for the main queue

        // Setup Swapchain
        CheckAndConfigureSwapChainSupport(); // Configure swap chain
        CreateSwapChain(); // Create swap chain and image views
        CreateCommandBuffers(); // Create Command Pool and Allocate buffers

        // Sync Objects
        CreateSyncObjects();
	}

    Window::~Window()
    {
        // Destroy Sync objects
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_VKDeviceHandle->GetVKDevice(), m_VKImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VKDeviceHandle->GetVKDevice(), m_VKRenderCompleteSemaphores[i], nullptr);
            vkDestroyFence(m_VKDeviceHandle->GetVKDevice(), m_VKInFlightFences[i], nullptr);
        }

        // Destroy command pool
        if (m_VKCommandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_VKDeviceHandle->GetVKDevice(), m_VKCommandPool, nullptr);

        // Destroy all image view
        for (auto imageView : m_VKSwapChainImageViews)
        {
            vkDestroyImageView(m_VKDeviceHandle->GetVKDevice(), imageView, nullptr);
        }

        // Destroy Swap Chain
        vkDestroySwapchainKHR(m_VKDeviceHandle->GetVKDevice(), m_VKSwapChain, nullptr);

        // Destroy Surface
        if (m_VKSurface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_VKInstanceHandle->GetVkInstance(), m_VKSurface, nullptr);
    }

    // Create Vulkan Surface
    void Window::CreateSurface()
    {
        auto VKCreateWin32Surface =
            (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_VKInstanceHandle->GetVkInstance(), "vkCreateWin32SurfaceKHR");

        if (!VKCreateWin32Surface)
        {
            Logger::Log_Error("Unable to create surface. Win32 surface extension not available!");
            throw std::runtime_error("Unable to create surface. Win32 surface extension not available!");
        }

        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = m_hWND;
        createInfo.hinstance = m_hInstance;

        if (vkCreateWin32SurfaceKHR(m_VKInstanceHandle->GetVkInstance(), &createInfo, nullptr, &m_VKSurface) != VK_SUCCESS) {
            Logger::Log_Error("Unable to create surface. Failed to create window surface");
            throw std::runtime_error("Unable to create surface. Failed to create window surface");
        }
    }

    // Check if physical device supports presentation on main queue
    void Window::CheckPresentationSupport()
    {
        // Check if the queue family of a device supports presentation
        VkBool32 presentSupport{};
        auto error{ vkGetPhysicalDeviceSurfaceSupportKHR
        (
          m_VKDeviceHandle->GetVKPhysicalDevice(), m_VKDeviceHandle->GetMainQueueIndex(),
          m_VKSurface, &presentSupport)
        };
        if (error || presentSupport == VK_FALSE)
        {
            Logger::Log_Error("Unable to create Surface. Physical device has no queue support for Surface presentation!");
            throw std::runtime_error("Unable to create Surface. Physical device has no queue support for Surface presentation!");
        }
    }

    void Window::CheckAndConfigureSwapChainSupport()
    {
        // Check for Swap Chain support in Device
        /*
            3 Things to check for
                a. Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
                b. Surface formats (pixel format, color space)
                c. Available presentation modes
        */

        // Get Surface formats
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_VKDeviceHandle->GetVKPhysicalDevice(), m_VKSurface, &m_VKSurfaceCapabilities);
        uint32_t formatCount{ 0 };
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_VKDeviceHandle->GetVKPhysicalDevice(), m_VKSurface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_VKDeviceHandle->GetVKPhysicalDevice(), m_VKSurface, &formatCount, formats.data());

        // Get Presentation Modes
        uint32_t presentModeCount{ 0 };
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_VKDeviceHandle->GetVKPhysicalDevice(), m_VKSurface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_VKDeviceHandle->GetVKPhysicalDevice(), m_VKSurface, &presentModeCount, presentModes.data());

        // No format or presentation mode avail -> Device not swap chain supported
        if (formatCount == 0 || presentModeCount == 0)
        {
            Logger::Log_Error("Unable to create Swap Chain. Device is not Swap Chain supported.");
            throw std::runtime_error("Unable to create Swap Chain. Device is not Swap Chain supported.");
        }

        // Configure Swap Chain
        // Get preferred surface format (Format + Color Space)
        for (auto format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                m_VKSurfaceFormat = format;
                break;
            }
        }

        // Set presentation mode based on vsync
        m_VKPresentMode = m_VSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    void Window::CreateSwapChain()
    {

        // CREATE SWAP CHAIN
        // Request 1 more image than minimum to account for waiting on driver's internal operations to complete
        uint32_t imageCount{ m_VKSurfaceCapabilities.minImageCount + 1 };

        // Ensure that we do not exceed max image count
        if (m_VKSurfaceCapabilities.maxImageCount > 0 && imageCount > m_VKSurfaceCapabilities.maxImageCount)
            imageCount = m_VKSurfaceCapabilities.maxImageCount;

        // Swap chain create info
        VkSwapchainCreateInfoKHR swapChainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_VKSurface,
            .minImageCount = imageCount,
            .imageFormat = m_VKSurfaceFormat.format,
            .imageColorSpace = m_VKSurfaceFormat.colorSpace,
            .imageExtent = m_VKSurfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = m_VKPresentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        // Create swap chain
        if (auto VkErr = vkCreateSwapchainKHR(m_VKDeviceHandle->GetVKDevice(), &swapChainCreateInfo, nullptr, &m_VKSwapChain); VkErr)
        {
            Logger::Log_Error("Unable to create Swap Chain. VKCreateSwapChainKHR failed");
            throw std::runtime_error("Unable to create Swap Chain. VKCreateSwapChainKHR failed");
        }

        // Retrieve Images
        vkGetSwapchainImagesKHR(m_VKDeviceHandle->GetVKDevice(), m_VKSwapChain, &imageCount, nullptr);
        m_VKSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_VKDeviceHandle->GetVKDevice(), m_VKSwapChain, &imageCount, m_VKSwapChainImages.data());

        // Set variables to be used later...
        m_VKSwapChainImageFormat = m_VKSurfaceFormat.format;
        m_VKSwapExtent = m_VKSurfaceCapabilities.currentExtent;

        // CREATE IMAGE VIEWS
        m_VKSwapChainImageViews.resize(m_VKSwapChainImages.size());
        for (int i{ 0 }; i < m_VKSwapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = m_VKSwapChainImages[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = m_VKSwapChainImageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            if (auto VkErr{ vkCreateImageView(m_VKDeviceHandle->GetVKDevice(), &imageViewCreateInfo, nullptr, &m_VKSwapChainImageViews[i])}; VkErr)
            {
                Logger::Log_Error("Unable to create Swap Chain. Failed to create Image Views");
                throw std::runtime_error("Unable to create Swap Chain. Failed to create Image Views");
            }
        }

    }

    void Window::CreateCommandBuffers()
    {
        // Create Command Pool
        VkCommandPoolCreateInfo commandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // All command buffers are resettable together
            .queueFamilyIndex = m_VKDeviceHandle->GetMainQueueIndex()
        };

        if (auto VkErr{ vkCreateCommandPool(m_VKDeviceHandle->GetVKDevice(), &commandPoolCreateInfo, nullptr, &m_VKCommandPool)}; VkErr)
        {
            Logger::Log_Error("Unable to create Command Pool. vkCreateCommandPool failed.");
            throw std::runtime_error("Unable to create Command Pool. vkCreateCommandPool failed.");
        }

        // Allocate as many Command Buffers as Framebuffers(same number as image views) -> Each Buffer will work on a specified frame buffer based on index
        m_VKCommandBuffers.resize(m_VKSwapChainImageViews.size());

        // Describe command buffers
        VkCommandBufferAllocateInfo commandBufferAllocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_VKCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // Currently all are primary cmd buffers
            .commandBufferCount = static_cast<uint32_t>(m_VKCommandBuffers.size())
        };

        if (auto VkErr{ vkAllocateCommandBuffers(m_VKDeviceHandle->GetVKDevice(), &commandBufferAllocInfo, m_VKCommandBuffers.data())}; VkErr)
        {
            Logger::Log_Error("Unable to allocate Command Buffers. vkAllocateCommandBuffer failed.");
            throw std::runtime_error("Unable to allocate Command Buffers. vkAllocateCommandBuffer failed.");
        }
    }

    void Window::CreateSyncObjects()
    {
        // Set vectors to contain enough semaphores and fences for max no. of in flight frames
        m_VKImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_VKRenderCompleteSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_VKInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        // Describe Semaphores
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // Describe Fences
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(m_VKDeviceHandle->GetVKDevice(), &semaphoreInfo, nullptr, &m_VKImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_VKDeviceHandle->GetVKDevice(), &semaphoreInfo, nullptr, &m_VKRenderCompleteSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_VKDeviceHandle->GetVKDevice(), &fenceInfo, nullptr, &m_VKInFlightFences[i]) != VK_SUCCESS)
            {
                Logger::Log_Error("Unable to create Sync Objects for a frame.");
                throw std::runtime_error("Unable to create Sync Objects for a frame.");
            }
        }

    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_DESTROY:
        {
            DestroyWindow(hWnd);
            PostQuitMessage(0);
        }
        break;
        case WM_PAINT:
        {
            ValidateRect(hWnd, NULL);
        }
        break;
        case WM_SIZE:
        {
            if (auto pWin = reinterpret_cast<Minerva::Vulkan::Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); pWin)
            {
                if (wParam != SIZE_MINIMIZED)
                {
                    pWin->SetWindowValues(static_cast<int>(lParam & 0xffff), static_cast<int>((lParam & 0xffff0000) >> 16), false);
                    return 0;
                }
                else
                {
                    std::cout << pWin->GetDeviceHandle()->GetVKInstanceHandle()->GetApplicationName() << "\n";
                    pWin->SetMinimized(true);
                }
            }
        }
        break;

        case WM_KEYUP:
        {
            Minerva::Input::UpdateKeyUpCallback(static_cast<Minerva::Keycode>(wParam));
        }
        break;
        case WM_MOUSEMOVE:
        {
            Minerva::Input::UpdateMousePositionCallback(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
        }
        

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

	void CreateWindowClass(HINSTANCE _hInstance, WNDPROC _wndproc)
	{
        //
        // Check if we already register the class
        //

        WNDCLASSA C{};
        if (GetClassInfoA(_hInstance, "MinervaClass", &C))
        {
            Logger::Log_Error("Unable to create Window. Failed to get class info");
            throw std::runtime_error("Unable to create Window. Failed to get class info");
        }


        WNDCLASSEX wndClass;
        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = _wndproc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = _hInstance;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = TEXT("MinervaClass");
        wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

        if (!RegisterClassEx(&wndClass))
        {
            Logger::Log_Error("Unable to create Window. Failed to register info class");
            throw std::runtime_error("Unable to create Window. Failed to register info class");
        }
	}
    void CreateSystemWindow(HINSTANCE  _hInstance, HWND& _hWnd, bool _bFullScreen, int _width, int _height)
    {
        //
        // Get Resolution
        //
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        if (_bFullScreen)
        {
            DEVMODE dmScreenSettings{};
            memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
            dmScreenSettings.dmSize = sizeof(dmScreenSettings);
            dmScreenSettings.dmPelsWidth = screenWidth;
            dmScreenSettings.dmPelsHeight = screenHeight;
            dmScreenSettings.dmBitsPerPel = 32;
            dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            if ((_width != screenWidth) && (_height != screenHeight))
            {
                if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                {
                    Logger::Log_Error("Fullscreen Mode not supported!");
                    throw std::runtime_error("Fullscreen Mode not supported!");
                }
            }
        }

        //
        // Compute windows flags
        //
        const DWORD dwExStyle = _bFullScreen ? WS_EX_APPWINDOW : WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        const DWORD dwStyle = _bFullScreen ? WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN : WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        //
        // Determine window rectangle
        //
        RECT windowRect;
        if (_bFullScreen)
        {
            windowRect.left = static_cast<long>(0);
            windowRect.right = static_cast<long>(screenWidth);
            windowRect.top = static_cast<long>(0);
            windowRect.bottom = static_cast<long>(screenHeight);
        }
        else
        {
            windowRect.left = static_cast<long>(screenWidth) / 2 - _width / 2;
            windowRect.right = static_cast<long>(_width);
            windowRect.top = static_cast<long>(screenHeight) / 2 - _height / 2;
            windowRect.bottom = static_cast<long>(_height);
        }

        AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

        //
        // Create Window
        //
        _hWnd = CreateWindowEx
        (
            0
            , TEXT("MinervaClass")
            , TEXT("Minerva")
            , dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
            , windowRect.left
            , windowRect.top
            , windowRect.right
            , windowRect.bottom
            , NULL
            , NULL
            , _hInstance
            , NULL
        );

        if (!_hWnd)
        {
            Logger::Log_Error("Unable to create window. Failed to create window handle");
            throw std::runtime_error("Unable to create window. Failed to create window handle");
        }

        ShowWindow(_hWnd, SW_SHOW);
        SetForegroundWindow(_hWnd);
        SetFocus(_hWnd);
    }


    Minerva::Window::RenderStatus Window::BeginRender(std::shared_ptr<Minerva::Vulkan::Renderpass> _renderpass)
    {
        if (m_Minimized) { return Minerva::Window::RenderStatus::WINDOW_MINIMIZED; }

        // Set Renderpass and pipeline being used
        m_VKRenderpassHandle = _renderpass;

        // Wait for previous frame to be finished
        vkWaitForFences(m_VKDeviceHandle->GetVKDevice(), 1, &m_VKInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        // Acquire Swap Chain index, will use this to index the framebuffer
        
        VkResult result = vkAcquireNextImageKHR(m_VKDeviceHandle->GetVKDevice(), m_VKSwapChain, UINT64_MAX, m_VKImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

        // Check if window resized
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            //RecreateSwapchain(m_VKRenderpassHandle);
            return Minerva::Window::RenderStatus::WINDOW_RESIZED;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            Logger::Log_Error("Window resized but Suboptimal");
            throw std::runtime_error("Window resized but Suboptimal");
        }

        // Reset fence to be reused
        vkResetFences(m_VKDeviceHandle->GetVKDevice(), 1, &m_VKInFlightFences[m_CurrentFrame]);

        // Reset command buffer
        vkResetCommandBuffer(m_VKCommandBuffers[m_CurrentFrame] /*VkCommandBufferResetFlagBits*/, 0);

        return Minerva::Window::RenderStatus::RENDER_OK;
    }

    Minerva::Window::RenderStatus Window::PageFlip()
    {
        Minerva::Window::RenderStatus retval{ Minerva::Window::RenderStatus::RENDER_OK };
        // End Render Pass
        vkCmdEndRenderPass(m_VKCommandBuffers[m_CurrentFrame]);
        // End Command Buffer
        if (vkEndCommandBuffer(m_VKCommandBuffers[m_CurrentFrame]) != VK_SUCCESS) {
            Logger::Log_Error("Failed to record Command Buffer");
            throw std::runtime_error("Failed to record Command Buffer");
        }

        // Describe submit information
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        // Set image available semaphore
        VkSemaphore waitSemaphores[] = { m_VKImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        // Set command buffers to be submitted
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_VKCommandBuffers[m_CurrentFrame];
        // Set render complete semaphore
        VkSemaphore signalSemaphores[] = { m_VKRenderCompleteSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // SUBMIT QUEUE OPERATION
        if (vkQueueSubmit(m_VKDeviceHandle->GetMainQueue(), 1, &submitInfo, m_VKInFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
            Logger::Log_Error("Failed to submit draw command buffer");
            throw std::runtime_error("Failed to submit draw command buffer");
        }

        // PRESENTATION
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        // Set Render semaphore
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        // Set swap chain
        VkSwapchainKHR swapChains[] = { m_VKSwapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        // Set image index
        presentInfo.pImageIndices = &m_ImageIndex;

        // SUBMIT QUEUE
        VkResult result = vkQueuePresentKHR(m_VKDeviceHandle->GetMainQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Resized)
        {
            m_Resized = false;
            retval = Minerva::Window::RenderStatus::WINDOW_RESIZED;
            //RecreateSwapchain(m_VKRenderpassHandle);
        }
        else if (result != VK_SUCCESS)
        {
            Logger::Log_Error("Failed to present swap chain image");
            throw std::runtime_error("Failed to present swap chain image");
        }

        vkQueueWaitIdle(m_VKDeviceHandle->GetMainQueue());

        // Increment to next frame
        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return retval;
    }

    void Window::SetWindowValues(int _width, int _height, bool _isMinimized)
    {
        m_Width = _width;
        m_Height = _height;
        m_Resized = true;
        m_Minimized = _isMinimized;
    }

    void Window::CleanupSwapchain()
    {
        // Destroy all image view
        for (auto imageView : m_VKSwapChainImageViews)
        {
            vkDestroyImageView(m_VKDeviceHandle->GetVKDevice(), imageView, nullptr);
        }

        // Destroy Swap Chain
        vkDestroySwapchainKHR(m_VKDeviceHandle->GetVKDevice(), m_VKSwapChain, nullptr);
    }

    void Window::RecreateSwapchain()
    {
        CheckAndConfigureSwapChainSupport();
        CreateSwapChain();
    }

    Minerva::CommandBuffer Window::GetCommandBuffer()
    {
        return Minerva::CommandBuffer(m_VKRenderpassHandle, m_VKCommandBuffers[m_CurrentFrame], m_VKSwapExtent, m_ImageIndex);
    }
    
}
