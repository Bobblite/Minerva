#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

//! Required Libraries
#include <array>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string_view>
#include <source_location>
#include <span>
#include <sstream>
#include <string>
#include <memory>
#include <unordered_map>
#include <limits>
#include <vector>

//! Vulkan API
#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_win32.h>

//! Windows API
#include <Windows.h>
#include <windowsx.h>
#include <wincon.h>

//! GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

//! TinyDDSLoader
#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>

//! In-house DDS Loader
#include <Minerva_DDSLoader.h>


//! Forward declaration of private interface
namespace Minerva::Vulkan
{
	class Instance;
	class Device;
	class Input;
	class Window;
	class Renderpass;
	class Shader;
	class VertexDescriptor;
	class Texture;
	class DescriptorSet;
	class Pipeline;
	class Buffer;
	class CommandBuffer;
}

//! Public Interface
#include "Minerva_Instance.h"
#include "Minerva_Device.h"
#include "Minerva_Input.h"
#include "Minerva_Window.h"
#include "Minerva_Renderpass.h"
#include "Minerva_Shader.h"
#include "Minerva_Vertex_Descriptor.h"
#include "Minerva_Texture.h"
#include "Minerva_DescriptorSet.h"
#include "Minerva_Pipeline.h"
#include "Minerva_Buffer.h"
#include "Minerva_CmdBuffer.h"

//! Private Interface
#include "../Details/MinervaVulkan/minerva_vulkan.h"

//! Inline
#include "../Details/Minerva_Instance_Inline.h"
#include "../Details/Minerva_Device_Inline.h"
#include "../Details/Minerva_Input_Inline.h"
#include "../Details/Minerva_Window_Inline.h"
#include "../Details/Minerva_Renderpass_Inline.h"
#include "../Details/Minerva_Shader_Inline.h"
#include "../Details/Minerva_Vertex_Descriptor_Inline.h"
#include "../Details/Minerva_Texture_Inline.h"
#include "../Details/Minerva_DescriptorSet_Inline.h"
#include "../Details/Minerva_Pipeline_Inline.h"
#include "../Details/Minerva_Buffer_Inline.h"
#include "../Details/Minerva_CmdBuffer_Inline.h"

