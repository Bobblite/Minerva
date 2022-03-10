namespace Minerva::Vulkan
{
	Shader::Shader(std::shared_ptr<Minerva::Vulkan::Device> _device, const std::string_view _filepath, Minerva::Shader::Type _shaderType) :
		m_VKDeviceHandle{ _device }, m_VKShaderModule{ VK_NULL_HANDLE }, m_ShaderType{ _shaderType }, m_Filename{_filepath}
	{
		// Load shader file
		std::ifstream ifs(_filepath.data(), std::ios::ate | std::ios::binary);

		if (!ifs.is_open())
		{
			Logger::Log_Error("Unable to load shader. Failed to open file.");
			throw std::runtime_error("Unable to load shader. Failed to open file.");
		}

		size_t fileSize{ static_cast<size_t>(ifs.tellg()) };
		std::vector<char> buffer(fileSize);
		ifs.seekg(0);
		ifs.read(buffer.data(), fileSize);
		ifs.close();

		// Create shader module
		VkShaderModuleCreateInfo shaderModuleCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = buffer.size(),
			.pCode = reinterpret_cast<const uint32_t*>(buffer.data()),
		};

		if (auto VkErr{ vkCreateShaderModule(m_VKDeviceHandle->GetVKDevice(), &shaderModuleCreateInfo, nullptr, &m_VKShaderModule)}; VkErr)
		{
			Logger::Log_Error("Unable to create shader. Failed to create VKShaderModule");
			throw std::runtime_error("Unable to create shader. Failed to create VKShaderModule");
		}
	}

	Shader::~Shader()
	{
		if (m_VKShaderModule != VK_NULL_HANDLE)
			vkDestroyShaderModule(m_VKDeviceHandle->GetVKDevice(), m_VKShaderModule, nullptr);
	}
}
