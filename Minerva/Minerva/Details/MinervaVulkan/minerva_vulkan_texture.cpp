namespace Minerva::Vulkan
{
	Texture::Texture(std::shared_ptr<Minerva::Vulkan::Device> _device, std::string_view _filePath) :
        m_VKDeviceHandle{_device},
        m_VKImage{ VK_NULL_HANDLE }, m_VKImageView{ VK_NULL_HANDLE }, m_VKImageFormat{VK_FORMAT_UNDEFINED},
        m_VKImageMemory{VK_NULL_HANDLE}, m_VKSampler{ VK_NULL_HANDLE },
        m_Width{ 0 }, m_Height{ 0 }, m_MipLevels{}
	{
		// Load DDS
		Minerva::Tools::DDSLoader::Bitmap loadedBitmap{};
		Minerva::Tools::DDSLoader::DDSError ddsErr{ Minerva::Tools::DDSLoader::LoadDDS(loadedBitmap, _filePath) };
		if (ddsErr != Minerva::Tools::DDSLoader::DDSError::SUCCESS)
		{
			std::stringstream ss;
			ss << "Error loading DDS. " << Minerva::Tools::DDSLoader::GetErrorMessage(ddsErr);
			Logger::Log_Error(ss.str());
			throw std::runtime_error(ss.str());
		}

		// Set member variables
        m_VKImageFormat = ConvertFormat(loadedBitmap.m_Format, loadedBitmap.m_ColorSpace, loadedBitmap.m_Signedness);
		m_MipLevels = loadedBitmap.m_MipLevels;
        m_Width = loadedBitmap.m_Width;
        m_Height = loadedBitmap.m_Height;

        //! Create staging buffer
        VkBuffer stagingBuffer{ VK_NULL_HANDLE };
        VkDeviceMemory imageMemory{ VK_NULL_HANDLE };

        CreateBuffer(loadedBitmap.m_Data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, imageMemory);

        void* data;
        vkMapMemory(m_VKDeviceHandle->GetVKDevice(), imageMemory, 0, loadedBitmap.m_Data.size(), 0, &data);
        memcpy(data, loadedBitmap.m_Data.data(), loadedBitmap.m_Data.size());
        vkUnmapMemory(m_VKDeviceHandle->GetVKDevice(), imageMemory);

        //! Create image
        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_VKImageFormat,
            .mipLevels = m_MipLevels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;

        //! Create vkImage
        if (int vkErr{ vkCreateImage(m_VKDeviceHandle->GetVKDevice(), &imageInfo, nullptr, &m_VKImage) }; vkErr)
        {
            Logger::Log_Error("Error Creating Texture. vkCreateImage() error.");
            throw std::runtime_error("Error Creating Texture. vkCreateImage() error.");
        }

        //! Allocate memory for image
        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(m_VKDeviceHandle->GetVKDevice(), m_VKImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };

        if (int vkErr{ vkAllocateMemory(m_VKDeviceHandle->GetVKDevice(), &allocInfo, nullptr, &m_VKImageMemory) }; vkErr)
        {
            Logger::Log_Error("Error Creating Texture. vkAllocateMemory() error.");
            throw std::runtime_error("Error Creating Texture. vkAllocateMemory() error.");
        }

        vkBindImageMemory(m_VKDeviceHandle->GetVKDevice(), m_VKImage, m_VKImageMemory, 0);

        TransitionImageLayout(m_VKImage, m_VKImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, m_VKImage, m_Width, m_Height);
        TransitionImageLayout(m_VKImage, m_VKImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        vkDestroyBuffer(m_VKDeviceHandle->GetVKDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_VKDeviceHandle->GetVKDevice(), imageMemory, nullptr);

        //! Create Image View
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_VKImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_VKImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_MipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (int vkErr{ vkCreateImageView(m_VKDeviceHandle->GetVKDevice(), &viewInfo, nullptr, &m_VKImageView) }; vkErr)
        {
            Logger::Log_Error("Unable to create Texture. vkCreateImageView() error.");
            throw std::runtime_error("Unable to create Texture. vkCreateImageView() error.");
        }

        //! Create Texture Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        //! Abstract this even further for more optionality
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_VKDeviceHandle->GetVKPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color when sampling with clamp
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (int vkErr{ vkCreateSampler(m_VKDeviceHandle->GetVKDevice(), &samplerInfo, nullptr, &m_VKSampler) }; vkErr)
        {
            Logger::Log_Error("Unable to create texture. VkCreateSampler() failed.");
            throw std::runtime_error("Unable to create texture. VkCreateSampler() failed.");
        }
	}

    Texture::~Texture()
    {
        vkDestroySampler(m_VKDeviceHandle->GetVKDevice(), m_VKSampler, nullptr);
        vkDestroyImageView(m_VKDeviceHandle->GetVKDevice(), m_VKImageView, nullptr);
        vkDestroyImage(m_VKDeviceHandle->GetVKDevice(), m_VKImage, nullptr);
        vkFreeMemory(m_VKDeviceHandle->GetVKDevice(), m_VKImageMemory, nullptr);
    }

    uint32_t Texture::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_VKDeviceHandle->GetVKPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Texture::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
    {
        // Describe buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = _size;
        bufferInfo.usage = _usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_VKDeviceHandle->GetVKDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) {
            Logger::Log_Error("Unable to create buffer. vkCreateBuffer failed.");
            throw std::runtime_error("Unable to create buffer. vkCreateBuffer failed.");
        }

        // Query memory requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_VKDeviceHandle->GetVKDevice(), _buffer, &memRequirements);

        // Describe memory allocation
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, _properties);

        // Allocate memory
        if (vkAllocateMemory(m_VKDeviceHandle->GetVKDevice(), &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS)
        {
            Logger::Log_Error("Unable to create Buffer. Failed to allocate buffer memory.");
            throw std::runtime_error("Unable to create Buffer. Failed to allocate buffer memory.");
        }

        vkBindBufferMemory(m_VKDeviceHandle->GetVKDevice(), _buffer, _bufferMemory, 0);
    }

    void Texture::TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout)
    {
        VkCommandBuffer commandBuffer{ m_VKDeviceHandle->BeginSingleTimeCommands() };

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = _oldLayout;
        barrier.newLayout = _newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = _image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        // Transition barrier masks
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            Logger::Log_Error("Error setting up Image Barrier. Unsupported layout transition.");
            throw std::runtime_error("Error setting up Image Barrier. Unsupported layout transition.");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        m_VKDeviceHandle->EndSingleTimeCommands(commandBuffer);
    }

    void Texture::CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height)
    {
        VkCommandBuffer commandBuffer{ m_VKDeviceHandle->BeginSingleTimeCommands() };

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            _width,
            _height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        m_VKDeviceHandle->EndSingleTimeCommands(commandBuffer);
    }

    VkFormat Texture::ConvertFormat(Minerva::Tools::PixelFormat::ImageFormat _format,
        Minerva::Tools::PixelFormat::ColorSpace _colorspace,
        Minerva::Tools::PixelFormat::Signedness _signedness)
    {
        using namespace Minerva::Tools::PixelFormat;
        std::tuple<VkFormat, VkFormat, VkFormat> possibleVKFormats;

        switch (_format)
        {
            //! Store all possible VKFormats according to ImageFormat _format
            //! LINEAR UNSIGNED | LINEAR SIGNED | SRGB
            case ImageFormat::BC1_4RGBA1:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_BC1_RGBA_SRGB_BLOCK);
            } break;
            case ImageFormat::BC2_8RGBA:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_BC2_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_BC2_SRGB_BLOCK);
            } break;
            case ImageFormat::BC3_8RGBA:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_BC3_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_BC3_SRGB_BLOCK);
            } break;
            case ImageFormat::R8G8B8A8:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SRGB);
            } break;
            case ImageFormat::B8G8R8A8:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SRGB);
            } break;
            case ImageFormat::B8G8R8U8:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM, VK_FORMAT_B8G8R8A8_SRGB);
            } break;
            case ImageFormat::BC5_8RG:
            {
                possibleVKFormats = std::make_tuple(VK_FORMAT_BC5_UNORM_BLOCK, VK_FORMAT_BC5_SNORM_BLOCK, VK_FORMAT_UNDEFINED);
            } break;
        }

        //! Filter actual format type on ColorSpace or Signedness
        // Linear space
        if (_colorspace == ColorSpace::LINEAR)
        {
            // Signedness
            if (_signedness == Signedness::UNSIGNED)
                return std::get<0>(possibleVKFormats);
            else
                return std::get<1>(possibleVKFormats);
        }
        // SRGB
        else
            return std::get<2>(possibleVKFormats);
    }
}