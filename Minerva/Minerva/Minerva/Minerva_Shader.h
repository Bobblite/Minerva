#pragma once

namespace Minerva
{
	class Shader
	{
	public:
		enum class Type : uint8_t
		{
			VERTEX,
			FRAGMENT,
			TESSELLATION,
			COMPUTE,
			ENUM_COUNT
		};

		Shader(Minerva::Device& _device, const std::string_view _filepath, Type _shaderType);

		inline std::shared_ptr<Minerva::Vulkan::Shader> GetVKShaderHandle() const { return m_VKShaderHandle; }

		inline Minerva::Shader::Type GetShaderType() const;
		inline std::string_view GetFilepath() const;

	private:
		// Private interface handle
		std::shared_ptr<Minerva::Vulkan::Shader> m_VKShaderHandle;
	};
}
