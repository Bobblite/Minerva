#pragma once

namespace Minerva
{
	class DescriptorSet
	{
	public:

		enum class DescriptorType : uint8_t
		{
			SAMPLER = 0,
			COMBINED_IMAGE_SAMPLER,
			SAMPLED_IMAGE,
			STORAGE_IMAGE,
			UNIFORM_TEXEL_BUFFER,
			STORAGE_TEXEL_BUFFER,
			UNIFORM_BUFFER,
			STORAGE_BUFFER,
			UNIFORM_BUFFER_DYNAMIC,
			STORAGE_BUFFER_DYNAMIC,
			INPUT_ATTACHMENT
		};

		struct Layout
		{
			uint32_t m_BindingPoint;
			DescriptorType m_DescriptorType;
			uint32_t m_DescriptorCount;
			Shader::Type m_ShaderStage;
		};

		DescriptorSet(Minerva::Device &_device, std::span<Minerva::DescriptorSet::Layout> _layouts);
		~DescriptorSet();

		inline std::shared_ptr<Minerva::Vulkan::DescriptorSet> GetVKDescriptorSetHandle() const;

		inline void Update(const Layout& _layout, std::span<Minerva::Texture> _textures);

	private:
		std::shared_ptr<Minerva::Vulkan::DescriptorSet> m_VKDescriptorSetHandle;
	};
}
