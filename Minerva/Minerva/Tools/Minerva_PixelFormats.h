#pragma once
#include <tinyddsloader.h>
#include <cstdint>

namespace Minerva::Tools::PixelFormat
{
	using namespace tinyddsloader;
		
	enum class ImageFormat: uint8_t
	{
		BC1_4RGBA1 = 0,
		BC2_8RGBA,
		BC3_8RGBA,
		R8G8B8A8,
		B8G8R8A8,
		B8G8R8U8,
		BC5_8RG
	};

	enum class ColorSpace : uint8_t
	{
		LINEAR = 0,
		SRGB
	};

	enum class Signedness : uint8_t
	{
		UNSIGNED = 0,
		SIGNED
	};
}
