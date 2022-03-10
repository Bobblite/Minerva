namespace Minerva::Vulkan
{

	Buffer::Buffer(std::shared_ptr<Minerva::Vulkan::Device> _device, Minerva::Buffer::Type _type, const void* _data, uint32_t _size) :
        m_VKDeviceHandle{ _device }, m_VKBuffer{ VK_NULL_HANDLE }, m_VKMemory{ VK_NULL_HANDLE }, m_VKSize{_size}, m_Type{ _type }
	{
        // Get UsageType based on Minerva::Buffer::Type
        auto UsageType = [](auto UsageType) constexpr
        {
            switch (UsageType)
            {
            case Minerva::Buffer::Type::VERTEX:               return (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            case Minerva::Buffer::Type::INDEX:                return (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
            //case Minerva::Buffer::Type::UNIFORM:               return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
        }(m_Type);

        // Get Propeties based on Minerva::Buffer::Type
        auto Properties = [](auto Properties) constexpr
        {
            switch (Properties)
            {
            case Minerva::Buffer::Type::VERTEX:
            case Minerva::Buffer::Type::INDEX:                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                //case Minerva::Buffer::Type::UNIFORM:               return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            }
        }(m_Type);

        switch (m_Type)
        {
        case Minerva::Buffer::Type::VERTEX:
        case Minerva::Buffer::Type::INDEX:
        {
            VkBuffer stagingBuffer;
            VkDeviceMemory  stagingBufferMemory;
            CreateBuffer(m_VKSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);

            // Fill staging buffer
            void* data;
            vkMapMemory(m_VKDeviceHandle->GetVKDevice(), stagingBufferMemory, 0, m_VKSize, 0, &data);
            memcpy(data, _data, static_cast<size_t>(m_VKSize));
            vkUnmapMemory(m_VKDeviceHandle->GetVKDevice(), stagingBufferMemory);

            CreateBuffer(m_VKSize, UsageType, Properties,
                m_VKBuffer, m_VKMemory);

            m_VKDeviceHandle->CopyBuffer(stagingBuffer, m_VKBuffer, m_VKSize);
            
        }break;
        }
	}

    Buffer::~Buffer()
    {
        vkDestroyBuffer(m_VKDeviceHandle->GetVKDevice(), m_VKBuffer, nullptr);
        vkFreeMemory(m_VKDeviceHandle->GetVKDevice(), m_VKMemory, nullptr);
    }

    uint32_t Buffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

    void Buffer::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
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
}