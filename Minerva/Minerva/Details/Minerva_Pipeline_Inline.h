#pragma once

namespace Minerva
{
	Pipeline::Pipeline(Minerva::Device& _device, Minerva::Window& _window, Minerva::Renderpass& _renderpass, const Minerva::Shader* _shaders, int _shaderCount, Minerva::VertexDescriptor& _vertDesc) :
		m_VKPipelineHandle{ nullptr }
	{
		m_VKPipelineHandle = std::make_shared<Minerva::Vulkan::Pipeline>
			(_device.GetVKDeviceHandle(),
				_window.GetVKWindowHandle(),
				_renderpass.GetVKRenderpassHandle(),
				_shaders,
				_shaderCount,
			_vertDesc.GetVKVertexDescriptorHandle());
	}

	inline std::shared_ptr<Minerva::Vulkan::Pipeline> Pipeline::GetVKPipelineHandle() const { return m_VKPipelineHandle; }
}
