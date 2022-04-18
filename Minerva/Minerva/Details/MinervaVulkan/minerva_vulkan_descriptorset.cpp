namespace Minerva::Vulkan
{
	DescriptorSet::DescriptorSet(std::shared_ptr<Minerva::Vulkan::Device> _device, std::span<Minerva::DescriptorSet::Layout> _layouts) :
		m_VKDeviceHandle{ _device }
	{
		if (_layouts.size() == 0)
		{
			Logger::Log_Error("Unable to create Descriptor Set. Must have at least one layout.");
			throw std::runtime_error("Unable to create Descriptor Set. Must have at least one layout.");
		}

		// Internal function to get Shader stage flag from Minerva::Shader::Type
		auto GetShaderFlag = [](Minerva::Shader::Type _type)
		{
			switch (_type)
			{
			case Minerva::Shader::Type::VERTEX:
				return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
				break;

			case Minerva::Shader::Type::FRAGMENT:
				return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
				break;

			default:
				return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL_GRAPHICS;
				break;
			}
		};

		// Setup all layout bindings
		std::vector<VkDescriptorSetLayoutBinding> bindings(_layouts.size());

		for (int i{ 0 }; i < _layouts.size(); ++i)
		{
			bindings[i].binding = _layouts[i].m_BindingPoint;
			bindings[i].descriptorCount = _layouts[i].m_DescriptorCount;
			bindings[i].descriptorType = static_cast<VkDescriptorType>(_layouts[i].m_DescriptorType);
			bindings[i].pImmutableSamplers = nullptr;
			bindings[i].stageFlags = GetShaderFlag(_layouts[i].m_ShaderStage);
		}

		// Create DescriptorSetLayout
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};

		if (int vkErr{ vkCreateDescriptorSetLayout(m_VKDeviceHandle->GetVKDevice(), &descriptorSetLayoutInfo, nullptr, &m_VKDescriptorSetLayout) }; vkErr)
		{
			Logger::Log_Error("Unable to create Descriptor Set. vkCreateDescriptorSetLayout error.");
			throw std::runtime_error("Unable to create Descriptor Set. vkCreateDescriptorSetLayout error.");
		}

		// Allocate Descriptor Set
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = m_VKDeviceHandle->GetVKDescriptorPool(),
			.descriptorSetCount = 1,
			.pSetLayouts = &m_VKDescriptorSetLayout
		};

		if (int vkErr{ vkAllocateDescriptorSets(m_VKDeviceHandle->GetVKDevice(), &descriptorSetAllocateInfo, &m_VKDescriptorSet) }; vkErr)
		{
			Logger::Log_Error("Unable to create Descriptor Set. vkAllocateDescriptorSets error.");
			throw std::runtime_error("Unable to create Descriptor Set. vkAllocateDescriptorSets error.");
		}
	}

	DescriptorSet::~DescriptorSet()
	{
		if (m_VKDescriptorSetLayout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(m_VKDeviceHandle->GetVKDevice(), m_VKDescriptorSetLayout, nullptr);
	}

	void DescriptorSet::Update(const Minerva::DescriptorSet::Layout& _layout, std::span<std::shared_ptr<Minerva::Vulkan::Texture>> _textures)
	{
		//! Create VkDescriptorImageInfos for each Texture in the layout
		std::vector<VkDescriptorImageInfo> imageInfos(_layout.m_DescriptorCount);

		for (int i{ 0 }; i < _layout.m_DescriptorCount; ++i)
		{
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[i].imageView = _textures[i]->GetVKImageView();
			imageInfos[i].sampler = _textures[i]->GetVKSampler();
		}

		//! DescriptorWrite information
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_VKDescriptorSet;
		descriptorWrite.dstBinding = _layout.m_BindingPoint;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = imageInfos.size();
		descriptorWrite.pImageInfo = imageInfos.data();

		//! Write into descriptor set
		vkUpdateDescriptorSets(m_VKDeviceHandle->GetVKDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}
