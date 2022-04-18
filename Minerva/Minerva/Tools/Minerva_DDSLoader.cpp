#include "Minerva_DDSLoader.h"

namespace Minerva::Tools::DDSLoader
{
	std::tuple<Minerva::Tools::PixelFormat::ImageFormat, Minerva::Tools::PixelFormat::ColorSpace, Minerva::Tools::PixelFormat::Signedness>
		ConvertFormat(DDSFile::DXGIFormat _format)
	{
		switch (_format)
		{
			case DDSFile::DXGIFormat::BC1_UNorm:
				return std::tuple{ ImageFormat::BC1_4RGBA1, ColorSpace::LINEAR, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::BC1_UNorm_SRGB:
				return std::tuple{ ImageFormat::BC1_4RGBA1, ColorSpace::SRGB, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::BC2_UNorm:
				return std::tuple{ ImageFormat::BC2_8RGBA, ColorSpace::LINEAR, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
				return std::tuple{ ImageFormat::BC2_8RGBA, ColorSpace::SRGB, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::BC3_UNorm:
				return std::tuple{ ImageFormat::BC3_8RGBA, ColorSpace::LINEAR, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
				return std::tuple{ ImageFormat::BC3_8RGBA, ColorSpace::SRGB, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::R8G8B8A8_UNorm:
				return std::tuple{ ImageFormat::R8G8B8A8, ColorSpace::LINEAR, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:
				return std::tuple{ ImageFormat::R8G8B8A8, ColorSpace::SRGB, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::R8G8B8A8_SNorm:
				return std::tuple{ ImageFormat::R8G8B8A8, ColorSpace::LINEAR, Signedness::SIGNED };
			case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
				return std::tuple{ ImageFormat::B8G8R8A8, ColorSpace::LINEAR, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
				return std::tuple{ ImageFormat::B8G8R8A8, ColorSpace::SRGB, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::B8G8R8A8_Typeless:
				return std::tuple{ ImageFormat::B8G8R8A8, ColorSpace::LINEAR, Signedness::SIGNED };
			case DDSFile::DXGIFormat::B8G8R8X8_UNorm:
				return std::tuple{ ImageFormat::B8G8R8U8, ColorSpace::LINEAR, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::B8G8R8X8_UNorm_SRGB:
				return std::tuple{ ImageFormat::B8G8R8U8, ColorSpace::SRGB, Signedness::UNSIGNED };
			case DDSFile::DXGIFormat::B8G8R8X8_Typeless:
				return std::tuple{ ImageFormat::B8G8R8U8, ColorSpace::LINEAR, Signedness::SIGNED };
			case DDSFile::DXGIFormat::BC5_UNorm:
				return std::tuple{ ImageFormat::BC5_8RG, ColorSpace::LINEAR, Signedness::UNSIGNED };
			default:
				return {};
		}
	}

	std::string GetErrorMessage(DDSError _code)
	{
		switch (_code)
		{
		case DDSError::SUCCESS:
			return std::string{ "SUCCESS" };
		case DDSError::ERROR_FILE_OPEN:
			return std::string{ "ERROR_FILE_OPEN" };
		case DDSError::ERROR_READ:
			return std::string{ "ERROR_READ" };
		case DDSError::ERROR_MAGIC_WORD:
			return std::string{ "ERROR_MAGIC_WORD" };
		case DDSError::ERROR_SIZE:
			return std::string{ "ERROR_SIZE" };
		case DDSError::ERROR_VERIFY:
			return std::string{ "ERROR_VERIFY" };
		case DDSError::ERROR_NO_SUPPORT:
			return std::string{ "ERROR_NOT_SUPPORTED" };
		case DDSError::ERROR_NOT_VALID_DATA:
			return std::string{ "ERROR_INVALID_DATA" };
		default:
			return std::string{ "UNKNOWN_ERROR" };
		}
	}

	DDSError LoadDDS(Bitmap& _bitmap, DDSFile& _image)
	{
		// Make sure we can handle the texture
		if (_image.GetTextureDimension() == DDSFile::TextureDimension::Unknown
			|| _image.GetTextureDimension() == DDSFile::TextureDimension::Texture3D)
		{
			return DDSError::ERROR_SIZE;
		}

		// get basic information about the texture
		auto pixelFormat = ConvertFormat(_image.GetFormat());
		_bitmap.m_Format = std::get<0>(pixelFormat);
		_bitmap.m_ColorSpace = std::get<1>(pixelFormat);
		_bitmap.m_Signedness = std::get<2>(pixelFormat);

		//! Error checking for this?
		
		// Prepare memory
		const auto MipTableBytes = _image.GetMipCount() * sizeof(int32_t);
		const auto FaceByteSize = [&]
		{
			auto FaceByteSize = 0;

			// Mips should be organized from biggest to smallest
			std::uint32_t PrevW = 0xffffffff;
			for (std::uint32_t i = 0; i < _image.GetMipCount(); i++)
			{
				auto View = _image.GetImageData(i, 0);
				FaceByteSize += View->m_memSlicePitch;
				if (View->m_width >= PrevW)
				{
					return -1;
				}
				PrevW = View->m_width;
			}

			return FaceByteSize;
		}();
		if (FaceByteSize == -1) return DDSError::ERROR_NOT_VALID_DATA;

		const auto nSubFaces = _image.IsCubemap() ? 6u : 1u;
		const auto FrameByteSize = FaceByteSize * nSubFaces;
		const auto nFrames = _image.GetArraySize();
		const auto TotalByteSize = MipTableBytes + FrameByteSize * nFrames;

		//
		// Copy memory
		//
		auto Memory = std::make_unique<std::byte[]>(TotalByteSize);
		auto pMipOffset = reinterpret_cast<uint32_t*>(Memory.get());
		auto pFrame = reinterpret_cast<std::byte*>(&pMipOffset[_image.GetMipCount()]);

		// Set the very first offset
		*pMipOffset = 0;

		for (std::uint32_t iFrame = 0; iFrame < nFrames; ++iFrame)
		{
			for (std::uint32_t iSubFace = 0; iSubFace < nSubFaces; ++iSubFace)
			{
				auto TopMipView = _image.GetImageData(0, iFrame * iSubFace);
				for (std::uint32_t iMip = 0; iMip < _image.GetMipCount(); ++iMip)
				{
					auto View = _image.GetImageData(iMip, iFrame * iSubFace);
					auto ByteSize = View->m_memSlicePitch;

					// Set the offset of the next mip
					if (iFrame == 0 && iSubFace == 0)
					{
						if ((iMip + 1) < _image.GetMipCount()) pMipOffset[iMip + 1] = pMipOffset[iMip] + static_cast<int>(ByteSize);
					}
					else
					{
						if (pMipOffset[iMip] != static_cast<int>(ByteSize))
							return DDSError::ERROR_NOT_VALID_DATA;
					}

					// Make sure that the size formula follows what we expect
					if (std::max(1u, (TopMipView->m_height >> iMip)) != View->m_height
						|| std::max(1u, (TopMipView->m_width >> iMip)) != View->m_width)
					{
						return DDSError::ERROR_NOT_VALID_DATA;;
					}

					// Copy the mip data
					std::memcpy(&pFrame[pMipOffset[iMip] + iSubFace * FaceByteSize + FrameByteSize * iFrame], View->m_mem, ByteSize);
				}
			}
		}

		_bitmap.m_Width = _image.GetImageData(0, 0)->m_width;
		_bitmap.m_Height = _image.GetImageData(0, 0)->m_height;
		_bitmap.m_FrameSize = FrameByteSize;
		_bitmap.m_Data = { Memory.release(), TotalByteSize };
		_bitmap.m_MipLevels = _image.GetMipCount();
		_bitmap.m_Frames = nFrames;

		return DDSError::SUCCESS;
	}

	DDSError LoadDDS(Bitmap& _bitmap, std::string_view fileName)
	{
		DDSFile image;
		
		if (auto Err = image.Load(fileName.data()); Err)
			return static_cast<DDSError>(Err);

		return LoadDDS(_bitmap, image);
	}
}