#pragma once

namespace Minerva
{
    Window::Window(Minerva::Device& _device, bool _fullscreen, bool _vsync, int _width, int _height) :
        m_VKWindowHandle{ nullptr }
    {
        m_VKWindowHandle =
            std::make_shared<Minerva::Vulkan::Window>(
                _device.GetVKDeviceHandle(),
                _fullscreen,
                _vsync,
                _width,
                _height);
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

	inline std::shared_ptr<Minerva::Vulkan::Window> Window::GetVKWindowHandle() const { return m_VKWindowHandle; }

	inline void Window::SetFullscreen(bool _fullscreen) { m_VKWindowHandle->SetFullScreen(_fullscreen); }

	inline void Window::SetVSync(bool _vsync) { m_VKWindowHandle->SetVSync(_vsync); }

	inline int Window::GetWidth() const { return m_VKWindowHandle->GetWidth(); }

	inline int Window::GetHeight() const { return m_VKWindowHandle->GetHeight(); }

	inline void Window::SetWidth(int _width) { m_VKWindowHandle->SetWidth(_width); }

	inline void Window::SetHeight(int _height) { m_VKWindowHandle->SetHeight(_height); }

    inline Minerva::CommandBuffer Window::GetCommandBuffer()
    {
        return m_VKWindowHandle->GetCommandBuffer();
    }

    inline bool Window::BeginRender(Minerva::Pipeline& _pipeline)
    {
        RenderStatus retval{ m_VKWindowHandle->BeginRender(_pipeline.GetVKPipelineHandle()->GetVKRenderpassHandle())};
        if (retval == RenderStatus::WINDOW_RESIZED) // Deal with resize
        {
            std::cout << "Resize\n";
            vkDeviceWaitIdle(m_VKWindowHandle->GetDeviceHandle()->GetVKDevice());

            _pipeline.GetVKPipelineHandle()->GetVKRenderpassHandle()->CleanupRenderpass();
            //_pipeline.GetVKPipelineHandle()->CleanupPipeline();
            m_VKWindowHandle->CleanupSwapchain();

            m_VKWindowHandle->RecreateSwapchain();
            _pipeline.GetVKPipelineHandle()->GetVKRenderpassHandle()->RecreateRenderpass();
            //_pipeline.GetVKPipelineHandle()->RecreatePipeline();
            return true;
        }
        else if (retval == RenderStatus::WINDOW_MINIMIZED)
            return false;
        else if (retval == RenderStatus::RENDER_OK)
            return true;
    }

    inline void Window::PageFlip(Minerva::Pipeline& _pipeline)
    {
        RenderStatus retval{ m_VKWindowHandle->PageFlip() };
        if (retval == RenderStatus::WINDOW_RESIZED) // Deal with resize
        {
            _pipeline.GetVKPipelineHandle()->GetVKRenderpassHandle()->CleanupRenderpass();
           // _pipeline.GetVKPipelineHandle()->CleanupPipeline();
            m_VKWindowHandle->CleanupSwapchain();

            m_VKWindowHandle->RecreateSwapchain();
            _pipeline.GetVKPipelineHandle()->GetVKRenderpassHandle()->RecreateRenderpass();
            //_pipeline.GetVKPipelineHandle()->RecreatePipeline();
        }
    }
}
