namespace Minerva::Vulkan
{
    Renderpass::Renderpass(std::shared_ptr<Minerva::Vulkan::Device> _device, std::shared_ptr<Minerva::Vulkan::Window> _window, float* _clearColor) :
        m_VKDeviceHandle{ _device }, m_VKWindowHandle{ _window }, m_VKClearValue{ {_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]} },
        m_VKFramebufferExtent{ _window->GetVKSwapExtent() }
	{
        // CREATE RENDERPASS
        // Describe the color attachment
        VkAttachmentDescription colorAttachmentDescription{
        .format = _window->GetVKImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // Can be set to clear/not to clear
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        // Color attachment binding point/reference
        VkAttachmentReference colorAttachmentRef{
            .attachment = 0, // Index bound to color attachment
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        // Description of subpass -> Render pass must have at least 1 subpass
        VkSubpassDescription subpassDesc{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };

        // Subpass dependencies
        VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        // Describe render pass
        VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1, // Only color
        .pAttachments = &colorAttachmentDescription, // pointer to container of attachments
        .subpassCount = 1, // At least 1 subpass
        .pSubpasses = &subpassDesc, // pointer to array of subpass descriptions
        .dependencyCount = 1,
        .pDependencies = &dependency
        };

        // Create Render Pass
        if (auto VkErr{ vkCreateRenderPass(m_VKDeviceHandle->GetVKDevice(), &renderPassInfo, nullptr, &m_VKRenderPass)}; VkErr)
        {
            Logger::Log_Error("Unable to create render pass. vkCreateRenderPass failed.");
            throw std::runtime_error("Unable to create render pass. vkCreateRenderPass failed.");
        }


        // CREATE FRAMEBUFFERS
        // Framebuffers references the attachments used in renderpass.
        // Each swapchain image will require their own framebuffer
        // The Image used for the attachment will be the one returned by the swapchain
        //  - the output of the shader stages will be the image to be "drawn on"

        m_VKFramebuffers.resize(m_VKWindowHandle->GetVKSwapImageViews().size());
        for (size_t i{ 0 }; i < m_VKFramebuffers.size(); ++i)
        {
            VkImageView attachments[] = { m_VKWindowHandle->GetVKSwapImageViews()[i] };

            VkFramebufferCreateInfo frameBufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_VKRenderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = m_VKFramebufferExtent.width,
                .height = m_VKFramebufferExtent.height,
                .layers = 1
            };

            if (auto VkErr{ vkCreateFramebuffer(m_VKDeviceHandle->GetVKDevice(), &frameBufferCreateInfo, nullptr, &m_VKFramebuffers[i])})
            {
                Logger::Log_Error("Unable to create Framebuffer. vkCreateFramebuff failed.");
                throw std::runtime_error("Unable to create Framebuffer. vkCreateFramebuff failed.");
            }
        }
	}


    Renderpass::~Renderpass()
    {
        for (auto framebuffer : m_VKFramebuffers)
        {
            if (framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(m_VKDeviceHandle->GetVKDevice(), framebuffer, nullptr);
        }

        if (m_VKRenderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(m_VKDeviceHandle->GetVKDevice(), m_VKRenderPass, nullptr);
    }

    void Renderpass::CleanupRenderpass()
    {
        for (auto framebuffer : m_VKFramebuffers)
        {
            if (framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(m_VKDeviceHandle->GetVKDevice(), framebuffer, nullptr);
        }

        if (m_VKRenderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(m_VKDeviceHandle->GetVKDevice(), m_VKRenderPass, nullptr);
    }

    void Renderpass::RecreateRenderpass()
    {
        m_VKFramebufferExtent = m_VKWindowHandle->GetVKSwapExtent();

        // CREATE RENDERPASS
        // Describe the color attachment
        VkAttachmentDescription colorAttachmentDescription{
        .format = m_VKWindowHandle->GetVKImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // Can be set to clear/not to clear
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        // Color attachment binding point/reference
        VkAttachmentReference colorAttachmentRef{
            .attachment = 0, // Index bound to color attachment
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        // Description of subpass -> Render pass must have at least 1 subpass
        VkSubpassDescription subpassDesc{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };

        // Subpass dependencies
        VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        // Describe render pass
        VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1, // Only color
        .pAttachments = &colorAttachmentDescription, // pointer to container of attachments
        .subpassCount = 1, // At least 1 subpass
        .pSubpasses = &subpassDesc, // pointer to array of subpass descriptions
        .dependencyCount = 1,
        .pDependencies = &dependency
        };

        // Create Render Pass
        if (auto VkErr{ vkCreateRenderPass(m_VKDeviceHandle->GetVKDevice(), &renderPassInfo, nullptr, &m_VKRenderPass) }; VkErr)
        {
            Logger::Log_Error("Unable to create render pass. vkCreateRenderPass failed.");
            throw std::runtime_error("Unable to create render pass. vkCreateRenderPass failed.");
        }


        // CREATE FRAMEBUFFERS
        // Framebuffers references the attachments used in renderpass.
        // Each swapchain image will require their own framebuffer
        // The Image used for the attachment will be the one returned by the swapchain
        //  - the output of the shader stages will be the image to be "drawn on"

        m_VKFramebuffers.resize(m_VKWindowHandle->GetVKSwapImageViews().size());
        for (size_t i{ 0 }; i < m_VKFramebuffers.size(); ++i)
        {
            VkImageView attachments[] = { m_VKWindowHandle->GetVKSwapImageViews()[i] };

            VkFramebufferCreateInfo frameBufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_VKRenderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = m_VKFramebufferExtent.width,
                .height = m_VKFramebufferExtent.height,
                .layers = 1
            };

            if (auto VkErr{ vkCreateFramebuffer(m_VKDeviceHandle->GetVKDevice(), &frameBufferCreateInfo, nullptr, &m_VKFramebuffers[i]) })
            {
                Logger::Log_Error("Unable to create Framebuffer. vkCreateFramebuff failed.");
                throw std::runtime_error("Unable to create Framebuffer. vkCreateFramebuff failed.");
            }
        }
    }
}
