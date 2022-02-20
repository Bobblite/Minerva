#include "minerva_vulkan_window.h"
#include "minerva_vulkan_device.h"
#include "minerva_vulkan_instance.h"

#include <vulkan/vulkan_win32.h>

#include <vector>
#include <fstream>

namespace Minerva::Vulkan
{
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_DESTROY:
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            break;
        case WM_SIZE:
            if (auto pWin = reinterpret_cast<Minerva::Vulkan::Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); pWin)
            {
                if (wParam != SIZE_MINIMIZED)
                {
                    pWin->SetWindowValues(static_cast<int>((lParam & 0xffff0000) >> 16), static_cast<int>(lParam & 0xffff), false);
                    return 0;
                }
                else
                {
                    pWin->SetMinimized();
                }
            }
            break;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    Window::~Window()
    {
        CleanupSwapChain();

        // Destroy Semaphores
        for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (m_ImageAvailableSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_VKDeviceHandle->m_VKDevice, m_ImageAvailableSemaphores[i], nullptr);

            if (m_RenderFinishedSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_VKDeviceHandle->m_VKDevice, m_RenderFinishedSemaphores[i], nullptr);

            if (m_InFlightFences[i] != VK_NULL_HANDLE)
                vkDestroyFence(m_VKDeviceHandle->m_VKDevice, m_InFlightFences[i], nullptr);
        }
        

        // Destroy command pool
        if (m_CommandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_VKDeviceHandle->m_VKDevice, m_CommandPool, nullptr);

        // Destroy Surface
        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_VKInstanceHandle->m_vkInstance, m_Surface, nullptr);
        
        std::cout << "Destroying Minerva::Vulkan::Window\n";
    }

    void Window::Initialize(const Minerva::Vulkan::Window::Setup& _setup, std::shared_ptr<Minerva::Vulkan::Device> _pDevice)
    {
        
        m_VKDeviceHandle = _pDevice;
        m_VKInstanceHandle = m_VKDeviceHandle->m_Instance;
        m_VKClearValue[0] = VkClearValue{ .color = {.float32 = {_setup.m_ClearColorR,_setup.m_ClearColorG,_setup.m_ClearColorB,_setup.m_ClearColorA}} };
        m_bClearOnRender = _setup.m_bClearOnRender;
        m_bFullscreen = _setup.m_bFullScreen;
        m_bVSync = _setup.m_bVSyncOn;
        m_Width = _setup.m_Width;
        m_Height = _setup.m_Height;

        auto& Instance = m_VKDeviceHandle->m_Instance;
        const auto hInstance = GetModuleHandle(NULL);

        // Create OS Window
        CreateWindowClass(m_hInstance, WndProc);
        CreateSystemWindow(m_hInstance, m_hWND, m_bFullscreen, m_Width, m_Height);

        if (m_hWND) SetWindowLongPtr(m_hWND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        CreateSurface();
        CheckPresentationSupport();
        CheckAndConfigureSwapChainSupport();
        CreateSwapChain();
        CreateImageViews();

        //! PIPELINE
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFrameBuffers();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateSyncObjects();

    }

    bool Window::ProcessInput()
    {
        bool bRunning{ true };
        MSG Message;
        while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == Message.message)
            {
                bRunning = false;
                break;
            }
            else
            {
                TranslateMessage(&Message);
                DispatchMessageW(&Message);
            }
        }
        return bRunning;
    }

    void Window::DrawFrame()
    {
        if (m_bMinimized) { return; }

        vkWaitForFences(m_VKDeviceHandle->m_VKDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_VKDeviceHandle->m_VKDevice, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            m_VKInstanceHandle->LogError("Failed to acquire swap chain image");
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(m_VKDeviceHandle->m_VKDevice, 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

        m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

        /*VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
        };*/
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];
        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_VKDeviceHandle->m_VKDevice, 1, &m_InFlightFences[m_CurrentFrame]);

        if (auto VkErr{ vkQueueSubmit(m_VKDeviceHandle->m_VKMainQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to submit draw command buffer");
            throw std::runtime_error("Failed to submit draw command buffer");
        }

        

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores
            //.swapchainCount = 1,
            //.pSwapchains = swapChains,
            //.pImageIndices = &imageIndex,
        };

        VkSwapchainKHR swapChains[] = { m_SwapChain };

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; //Only have 1 swapchain atm

        result = vkQueuePresentKHR(m_VKDeviceHandle->m_VKMainQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_bResized)
        {
            m_bResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            m_VKInstanceHandle->LogError("Failed to present swap chain image");
            throw std::runtime_error("Failed to present swap chain image");
        }

        vkQueueWaitIdle(m_VKDeviceHandle->m_VKMainQueue);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Window::RecreateSwapChain()
    {
        vkDeviceWaitIdle(m_VKDeviceHandle->m_VKDevice);

        CleanupSwapChain();
        CheckAndConfigureSwapChainSupport();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFrameBuffers();
        CreateCommandBuffers();

        m_ImagesInFlight.resize(m_SwapChainImages.size(), VK_NULL_HANDLE);
    }

    void Window::CleanupSwapChain()
    {
        for (auto framebuffer : m_SwapChainFrameBuffers)
        {
            vkDestroyFramebuffer(m_VKDeviceHandle->m_VKDevice, framebuffer, nullptr);
        }

        vkFreeCommandBuffers(m_VKDeviceHandle->m_VKDevice, m_CommandPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());

        vkDestroyPipeline(m_VKDeviceHandle->m_VKDevice, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_VKDeviceHandle->m_VKDevice, m_PipelineLayout, nullptr);
        vkDestroyRenderPass(m_VKDeviceHandle->m_VKDevice, m_RenderPass, nullptr);

        for (auto imageView : m_SwapChainImageView)
        {
            vkDestroyImageView(m_VKDeviceHandle->m_VKDevice, imageView, nullptr);
        }

        m_ShaderStages.clear();

        vkDestroySwapchainKHR(m_VKDeviceHandle->m_VKDevice, m_SwapChain, nullptr);
    }

    int Window::CreateWindowClass(HINSTANCE hinstance, WNDPROC wndproc)
    {
        //
        // Check if we already register the class
        //

        WNDCLASSA C{};
        if (GetClassInfoA(hinstance, "MinervaClass", &C))
        {
            m_VKInstanceHandle->LogError("Failed to get class info");
            throw std::runtime_error("Failed to get class info");
        }
        

        WNDCLASSEX wndClass;
        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = WndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hinstance;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = TEXT("MinervaClass");
        wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

        if (!RegisterClassEx(&wndClass))
        {
            m_VKInstanceHandle->LogError("Failed to register info class");
            throw std::runtime_error("Failed to register info class");
        }

        return 0;
    }

    int Window::CreateSystemWindow(HINSTANCE  hInstance, HWND& hWnd, bool bFullScreen, int width, int height)
    {
        //
        // Get Resolution
        //
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        if (bFullScreen)
        {
            DEVMODE dmScreenSettings{};
            memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
            dmScreenSettings.dmSize = sizeof(dmScreenSettings);
            dmScreenSettings.dmPelsWidth = screenWidth;
            dmScreenSettings.dmPelsHeight = screenHeight;
            dmScreenSettings.dmBitsPerPel = 32;
            dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            if ((width != screenWidth) && (height != screenHeight))
            {
                if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                {
                    m_VKInstanceHandle->LogError("Fullscreen Mode not supported!");
                    throw std::runtime_error("Fullscreen Mode not supported!");
                }
            }
        }

        //
        // Compute windows flags
        //
        const DWORD dwExStyle = bFullScreen ? WS_EX_APPWINDOW : WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        const DWORD dwStyle = bFullScreen ? WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN : WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        //
        // Determine window rectangle
        //
        RECT windowRect;
        if (bFullScreen)
        {
            windowRect.left = static_cast<long>(0);
            windowRect.right = static_cast<long>(screenWidth);
            windowRect.top = static_cast<long>(0);
            windowRect.bottom = static_cast<long>(screenHeight);
        }
        else
        {
            windowRect.left = static_cast<long>(screenWidth) / 2 - width / 2;
            windowRect.right = static_cast<long>(width);
            windowRect.top = static_cast<long>(screenHeight) / 2 - height / 2;
            windowRect.bottom = static_cast<long>(height);
        }

        AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

        //
        // Create Window
        //
        hWnd = CreateWindowEx
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
            , hInstance
            , NULL
        );

        if (!hWnd)
            return -1;

        ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);
        SetFocus(hWnd);

        return 0;
    }

    int Window::CreateSurface()
    {
        auto VKCreateWin32Surface =
            (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_VKDeviceHandle->m_Instance->m_vkInstance, "vkCreateWin32SurfaceKHR");
        if (!VKCreateWin32Surface)
        {
           m_VKInstanceHandle->LogError("Win32 surface extension not available!");
           throw std::runtime_error("Win32 surface extension not available!");
        }

        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = m_hWND;
        createInfo.hinstance = m_hInstance;

        if (vkCreateWin32SurfaceKHR(m_VKDeviceHandle->m_Instance->m_vkInstance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS) {
            m_VKInstanceHandle->LogError("Failed to create window surface!");
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void Window::CheckPresentationSupport()
    {
        VkBool32 presentSupport{};
        auto error{ vkGetPhysicalDeviceSurfaceSupportKHR
        (
          m_VKDeviceHandle->m_VKPhysicalDevice, m_VKDeviceHandle->m_uMainQueueIndex,
          m_Surface, &presentSupport)
        };
        if (error || presentSupport == VK_FALSE)
            m_VKInstanceHandle->LogError("Physical device has no queue support for Surface presentation!");
    }

    void Window::CheckAndConfigureSwapChainSupport()
    {
        // Check for Swap chain support
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_VKDeviceHandle->m_VKPhysicalDevice, m_Surface, &m_SurfaceCapabilities);

        uint32_t formatCount{ 0 };
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_VKDeviceHandle->m_VKPhysicalDevice, m_Surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_VKDeviceHandle->m_VKPhysicalDevice, m_Surface, &formatCount, formats.data());

        uint32_t presentModeCount{ 0 };
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_VKDeviceHandle->m_VKPhysicalDevice, m_Surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentMode(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_VKDeviceHandle->m_VKPhysicalDevice, m_Surface, &presentModeCount, presentMode.data());

        if (!formatCount || !presentModeCount)
            m_VKInstanceHandle->LogError("Device is not Swap-Chain supported");

        // Configure Swap chain support

        // Get preferred format 
        for (auto& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                m_SurfaceFormat = format;
                break;
            }
        }

        // Set presentation mode
        if (!m_bVSync) m_PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        else m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    void Window::ChooseSwapExtent()
    {

    }

    void Window::CreateSwapChain()
    {
        // Request 1 more image than minimum for waiting on driver internal operations to complete
        uint32_t imageCount{ m_SurfaceCapabilities.minImageCount + 1};
        if (m_SurfaceCapabilities.maxImageCount > 0 && imageCount > m_SurfaceCapabilities.maxImageCount)
            imageCount = m_SurfaceCapabilities.maxImageCount;

        // Swap chain create info
        VkSwapchainCreateInfoKHR swapChainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_Surface,
            .minImageCount = imageCount,
            .imageFormat = m_SurfaceFormat.format,
            .imageColorSpace = m_SurfaceFormat.colorSpace,
            .imageExtent = m_SurfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = m_PresentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        // Create swap chain
        if (auto VkErr = vkCreateSwapchainKHR(m_VKDeviceHandle->m_VKDevice, &swapChainCreateInfo, nullptr, &m_SwapChain); VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to create Swap Chain");
            throw std::runtime_error("Failed to create Swap Chain");
        }

        // Retrieve Images
        vkGetSwapchainImagesKHR(m_VKDeviceHandle->m_VKDevice, m_SwapChain, &m_ImageCount, nullptr);
        m_SwapChainImages.resize(m_ImageCount);
        vkGetSwapchainImagesKHR(m_VKDeviceHandle->m_VKDevice, m_SwapChain, &m_ImageCount, m_SwapChainImages.data());

        // Set some other stuff
        m_swapChainImageFormat = m_SurfaceFormat.format;
        m_swapExtent = m_SurfaceCapabilities.currentExtent;
    }

    void Window::CreateImageViews()
    {
        m_SwapChainImageView.resize(m_SwapChainImages.size());
        for (int i{ 0 }; i < m_SwapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = m_SwapChainImages[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = m_swapChainImageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            if (auto VkErr{ vkCreateImageView(m_VKDeviceHandle->m_VKDevice, &imageViewCreateInfo, nullptr, &m_SwapChainImageView[i]) }; VkErr)
            {
                m_VKInstanceHandle->LogError("Failed to create Image Views!");
                throw std::runtime_error("Failed to create Image Views!");
            }
        }
    }

    void Window::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachmentDescription{
        .format = m_swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpassDesc{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };

        VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachmentDescription,
        .subpassCount = 1,
        .pSubpasses = &subpassDesc,
        .dependencyCount = 1,
        .pDependencies = &dependency
        };


        if (auto VkErr{ vkCreateRenderPass(m_VKDeviceHandle->m_VKDevice, &renderPassInfo, nullptr, &m_RenderPass) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to create render pass");
            throw std::runtime_error("Failed to create render pass");
        }

    }

    void Window::CreateGraphicsPipeline()
    {
        // Loading shaders from SPV to resource
        auto vertShaderData{ LoadShader("Assets/Shaders/vert.spv") };
        auto fragShaderData{ LoadShader("Assets/Shaders/frag.spv") };
        // Create shader modules
        VkShaderModule vertShaderModule{ CreateShaderModule(vertShaderData) };
        VkShaderModule fragShaderModule{ CreateShaderModule(fragShaderData) };

        // Storing shader stage info
        VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .pName = "main"
        };
        vertShaderStageCreateInfo.module = vertShaderModule;

        VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pName = "main"
        };
        fragShaderStageCreateInfo.module = fragShaderModule;

        m_ShaderStages.emplace_back(vertShaderStageCreateInfo);
        m_ShaderStages.emplace_back(fragShaderStageCreateInfo);

        // Vertex Input - Vertex descriptions
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
        };

        // Input Assembly - Type of geometry to be drawn + enabled primitive restart
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        // Viewport and scissors
        VkViewport viewport{
        .x = 0.f,
        .y = 0.f,
        .width = (float)m_swapExtent.width,
        .height = (float)m_swapExtent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f
        };

        VkRect2D scissors{};
        scissors.offset = { 0, 0 };
        scissors.extent = m_swapExtent;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissors
        };

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.f,
            .depthBiasClamp = 0.f,
            .depthBiasSlopeFactor = 0.f
        };

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        // Color Blending
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlendAttachmentStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState
        };

        // Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };

        if (auto VkErr{ vkCreatePipelineLayout(m_VKDeviceHandle->m_VKDevice, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to create Pipeline Layout");
            throw std::runtime_error("Failed to create Pipeline Layout");
        }

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = m_ShaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyCreateInfo,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizerCreateInfo,
            .pMultisampleState = &multisamplingCreateInfo,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlendAttachmentStateCreateInfo,
            .pDynamicState = nullptr,
            .layout = m_PipelineLayout,
            .renderPass = m_RenderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        if (auto VkErr{ vkCreateGraphicsPipelines(m_VKDeviceHandle->m_VKDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_Pipeline) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to create Graphics Pipeline");
            throw std::runtime_error("Failed to create Graphics Pipeline");
        }

        vkDestroyShaderModule(m_VKDeviceHandle->m_VKDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_VKDeviceHandle->m_VKDevice, vertShaderModule, nullptr);

    }

    void Window::CreateFrameBuffers()
    {
        m_SwapChainFrameBuffers.resize(m_SwapChainImageView.size());

        for (size_t i{ 0 }; i < m_SwapChainImageView.size(); ++i)
        {
            VkImageView attachments[] = { m_SwapChainImageView[i] };

            VkFramebufferCreateInfo frameBufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_RenderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = m_swapExtent.width,
                .height = m_swapExtent.height,
                .layers = 1
            };

            if (auto VkErr{ vkCreateFramebuffer(m_VKDeviceHandle->m_VKDevice, &frameBufferCreateInfo, nullptr, &m_SwapChainFrameBuffers[i]) })
            {
                m_VKInstanceHandle->LogError("Failed to create Framebuffers");
                throw std::runtime_error("Failed to create Framebuffers");
            }
        }
    }

    void Window::CreateCommandPool()
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_VKDeviceHandle->m_uMainQueueIndex
        };

        if (auto VkErr{ vkCreateCommandPool(m_VKDeviceHandle->m_VKDevice, &commandPoolCreateInfo, nullptr, &m_CommandPool) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to create Command Pool");
            throw std::runtime_error("Failed to create Command Pool");
        }

    }

    void Window::CreateCommandBuffers()
    {


        // Allocating command buffers
        m_CommandBuffers.resize(m_SwapChainFrameBuffers.size());
        VkCommandBufferAllocateInfo commandBufferAllocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size())
        };
        
        if (auto VkErr{ vkAllocateCommandBuffers(m_VKDeviceHandle->m_VKDevice, &commandBufferAllocInfo, m_CommandBuffers.data()) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to allocate Command Buffers");
            throw std::runtime_error("Failed to allocate Command Buffers");
        }

        // Command buffer recording
        for (size_t i{ 0 }; i < m_CommandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            };
            
            if (auto VkErr{ vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) }; VkErr)
            {
                m_VKInstanceHandle->LogError("Failed to begin recording Command Buffer");
                throw std::runtime_error("Failed to begin recording Command Buffer");
            }

            // Starting render pass
            VkRenderPassBeginInfo renderPassInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = m_RenderPass,
                .framebuffer = m_SwapChainFrameBuffers[i],
            };
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_swapExtent;
            renderPassInfo.clearValueCount = 1;
            VkClearValue clearColor = { {{0.f, 0.f, 0.f, 1.f}} };
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
            vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(m_CommandBuffers[i]);

            if (auto VkErr{ vkEndCommandBuffer(m_CommandBuffers[i]) }; VkErr)
            {
                m_VKInstanceHandle->LogError("Failed to record Command Buffers");
                throw std::runtime_error("Failed to record Command Buffers");
            }
        }

        
    }

    void Window::CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(m_SwapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            auto VkErrSem1{ vkCreateSemaphore(m_VKDeviceHandle->m_VKDevice, &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphores[i]) };
            auto VkErrSem2{ vkCreateSemaphore(m_VKDeviceHandle->m_VKDevice, &semaphoreCreateInfo, nullptr, &m_RenderFinishedSemaphores[i]) };
            auto VkErrFence{ vkCreateFence(m_VKDeviceHandle->m_VKDevice, &fenceCreateInfo, nullptr, &m_InFlightFences[i]) };
            if (VkErrSem1 || VkErrSem2 || VkErrFence)
            {
                m_VKInstanceHandle->LogError("Failed to create Synchronization Objects for a frame");
                throw std::runtime_error("Failed to create Synchronization Objects for a frame");
            }
        }

        
    }

    std::vector<char> Window::LoadShader(const std::string& _filePath)
    {
        std::ifstream ifs(_filePath, std::ios::ate | std::ios::binary);
        if (!ifs)
            throw std::runtime_error("Failed to load shader");

        size_t fileSize{ static_cast<size_t>(ifs.tellg()) };
        std::vector<char> buffer(fileSize);
        ifs.seekg(0);
        ifs.read(buffer.data(), fileSize);
        ifs.close();
        return buffer;
    }

    VkShaderModule Window::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t*>(code.data()),
        };

        VkShaderModule shaderModule;
        if (auto VkErr{ vkCreateShaderModule(m_VKDeviceHandle->m_VKDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) }; VkErr)
        {
            m_VKInstanceHandle->LogError("Failed to create shader module");
            throw std::runtime_error("Failed to create shader module");
        }

        return std::move(shaderModule);
    }

    void Window::SetWindowValues(int _width, int _height, bool _isMinimized)
    {
        m_Width = _width;
        m_Height = _height;
        m_bResized = true;
        m_bMinimized = _isMinimized;
    }

    void Window::SetMinimized()
    {
        m_bMinimized = true;
    }
}
