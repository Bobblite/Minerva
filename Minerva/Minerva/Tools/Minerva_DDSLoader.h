#pragma once
#include "Minerva_PixelFormats.h"
#include <tuple>
#include <string>
#include <span>

namespace Minerva::Tools::DDSLoader
{
	using namespace tinyddsloader;
	using namespace Minerva::Tools::PixelFormat;

	enum class DDSError : uint8_t
	{
		SUCCESS = 0,
		ERROR_FILE_OPEN,
		ERROR_READ,
		ERROR_MAGIC_WORD,
		ERROR_SIZE,
		ERROR_VERIFY,
		ERROR_NO_SUPPORT,
		ERROR_NOT_VALID_DATA
	};
	
	struct Bitmap
	{
		uint32_t m_Width;
		uint32_t m_Height;
		ImageFormat m_Format;
		ColorSpace m_ColorSpace;
		Signedness m_Signedness;
		uint64_t m_FrameSize;
		std::span<std::byte> m_Data;
		int m_MipLevels;
		int m_Frames;
	};

	std::tuple<Minerva::Tools::PixelFormat::ImageFormat, Minerva::Tools::PixelFormat::ColorSpace, Minerva::Tools::PixelFormat::Signedness>
		ConvertFormat(DDSFile::DXGIFormat _format);

	std::string GetErrorMessage(DDSError _code);

	DDSError LoadDDS(Bitmap& _bitmap, DDSFile& _image);

	DDSError LoadDDS(Bitmap& _bitmap, std::string_view fileName);
}