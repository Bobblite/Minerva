#pragma once

namespace Minerva
{
	Shader::Shader(Minerva::Device& _device, const std::string_view _filepath, Type _shaderType) :
		m_VKShaderHandle{ nullptr }
	{
		m_VKShaderHandle = std::make_shared<Minerva::Vulkan::Shader>(_device.GetVKDeviceHandle(), _filepath, _shaderType);
	}

	inline Minerva::Shader::Type Shader::GetShaderType() const { return m_VKShaderHandle->GetShaderType(); }

	inline std::string_view Shader::GetFilepath() const { return m_VKShaderHandle->GetFilepath(); }
}
