namespace Minerva::Vulkan
{
	VertexDescriptor::VertexDescriptor(std::span<Minerva::VertexDescriptor::Attribute> _attributes, uint32_t _vertexSize, Minerva::VertexDescriptor::Topology _topology) :
		m_VKTopology{}, m_VKInputStageCreateInfo{}, m_VKInputBindingDescriptions{}, m_VKInputAttributeDescriptions{}
	{
		// Resize vectors based on attributes
		m_VKInputBindingDescriptions.resize(_attributes.size());
		m_VKInputAttributeDescriptions.resize(_attributes.size());


		// Setup Binding Descriptions -> Tells Vulkan how to pass this data to the vertex shader once uploaded to GPU
		for (int i{ 0 }; i < _attributes.size(); ++i)
		{
			m_VKInputBindingDescriptions[i] = VkVertexInputBindingDescription{
				.binding = 0, //static_cast<uint32_t>(i),
				.stride = 0, // Stride of one (inputRate) to the next
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX // Consume per vertex
			};
		}

		// Set Attribute Description -> How to handle vertex inputs
		for (int i{ 0 }; i < _attributes.size(); ++i)
		{
			Minerva::VertexDescriptor::Attribute& attribute{ _attributes[i] };

			auto Format = [](auto Format) constexpr
			{
				switch (Format)
				{
				case Minerva::VertexDescriptor::Format::FLOAT_1D:               return std::pair{ VK_FORMAT_R32_SFLOAT,           4 };
				case Minerva::VertexDescriptor::Format::FLOAT_2D:               return std::pair{ VK_FORMAT_R32G32_SFLOAT,        8 };
				case Minerva::VertexDescriptor::Format::FLOAT_3D:               return std::pair{ VK_FORMAT_R32G32B32_SFLOAT,     12 };
				case Minerva::VertexDescriptor::Format::FLOAT_4D:               return std::pair{ VK_FORMAT_R32G32B32A32_SFLOAT,  16 };
				case Minerva::VertexDescriptor::Format::UINT8_1D_NORMALIZED:    return std::pair{ VK_FORMAT_R8_UNORM,             1 };
				case Minerva::VertexDescriptor::Format::UINT8_4D_NORMALIZED:    return std::pair{ VK_FORMAT_R8G8B8A8_UNORM,       4 };
				/*case Minerva::VertexDescriptor::Format::UINT8_1D:               return std::pair{ VK_FORMAT_R8_UINT,              1 };
				case Minerva::VertexDescriptor::Format::UINT16_1D:              return std::pair{ VK_FORMAT_R16_UINT,             2 };
				case Minerva::VertexDescriptor::Format::UINT32_1D:              return std::pair{ VK_FORMAT_R32_UINT,             4 };
				case Minerva::VertexDescriptor::Format::SINT8_3D_NORMALIZED:    return std::pair{ VK_FORMAT_R8G8B8_SNORM,         3 };*/
				}
				return std::pair{ VK_FORMAT_R32G32_SFLOAT, 0 };
			}(attribute.m_Format);

			m_VKInputAttributeDescriptions[i].binding = 0;
			m_VKInputAttributeDescriptions[i].location = static_cast<uint32_t>(i);
			m_VKInputAttributeDescriptions[i].offset = attribute.m_Offset;
			m_VKInputAttributeDescriptions[i].format = Format.first;

			m_VKInputBindingDescriptions[0].stride += Format.second; // Set stride
		}

		// Pipiline's Vertex Input state info
		m_VKInputStageCreateInfo = VkPipelineVertexInputStateCreateInfo
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
		, .vertexBindingDescriptionCount = 1
		, .pVertexBindingDescriptions = m_VKInputBindingDescriptions.data()
		, .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(_attributes.size())
		, .pVertexAttributeDescriptions = m_VKInputAttributeDescriptions.data()
		};

		// Topology to be used
		m_VKTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}
