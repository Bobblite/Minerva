#pragma once

namespace Minerva
{
	class Pipeline
	{
	public:

		enum class Type : uint8_t
		{
			GRAPHICS
		};

		Pipeline(Minerva::Device& _device, Minerva::Window& _window, Minerva::Renderpass& _renderpass, const Minerva::Shader* _shaders, int _shaderCount, Minerva::VertexDescriptor& _vertDesc);

		inline std::shared_ptr<Minerva::Vulkan::Pipeline> GetVKPipelineHandle() const;

	private:
		std::shared_ptr<Minerva::Vulkan::Pipeline> m_VKPipelineHandle;
	};
}
