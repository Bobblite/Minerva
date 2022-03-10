#pragma once

namespace Minerva
{
	class VertexDescriptor
	{
	public:
		// Available formats
		enum class Format : uint8_t
		{
			FLOAT_1D,
			FLOAT_2D,
			FLOAT_3D,
			FLOAT_4D,
			UINT8_1D_NORMALIZED,
			UINT8_4D_NORMALIZED
		};

		enum class Topology : uint8_t
		{
			TRIANGLE_LIST,
			POINT_LIST,
			LINE_LIST
		};

		// Describes binding of an Attribute
		struct Attribute
		{
			uint32_t m_Offset;
			Format m_Format;
		};

		VertexDescriptor(std::span<Attribute> _attributes, uint32_t _vertexSize, Topology _topology );

		inline std::shared_ptr<Minerva::Vulkan::VertexDescriptor> GetVKVertexDescriptorHandle() const;

	private:
		std::shared_ptr<Minerva::Vulkan::VertexDescriptor> m_VKVertexDescriptorHandle;
	};
}
