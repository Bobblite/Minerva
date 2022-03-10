#pragma once

namespace Minerva::Vulkan
{
	class Shader
	{
	public:
		Shader(std::shared_ptr<Minerva::Vulkan::Device> _device, const std::string_view _filepath, Minerva::Shader::Type _shaderType);
		~Shader();

		inline VkShaderModule GetVKShaderModule() const { return m_VKShaderModule; }
		inline Minerva::Shader::Type GetShaderType() const { return m_ShaderType; }
		inline std::string_view GetFilepath() const { return m_Filename; }

	private:
		// Private interface handles
		std::shared_ptr<Minerva::Vulkan::Device> m_VKDeviceHandle;

		// Vulkan properties
		VkShaderModule m_VKShaderModule;

		// Minerva properties
		Minerva::Shader::Type m_ShaderType;
		std::string m_Filename;
	};
}

#include "minerva_vulkan_shader.cpp"
