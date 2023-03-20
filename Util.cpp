#include <msclr/marshal_cppstd.h>
#include "Util.h"
#include "OpenCv.h"

#define IMAGE_FORMAT_PAIR(f1, f2)   ((uint32_t)(f1)) << 16 | ((uint32_t)(f2))

using namespace System;
using namespace System::Collections::ObjectModel;

using namespace msclr::interop;

namespace VRepair { namespace ImageProcessing { namespace OpenCV {
   
    static Util::Util()
    {
        auto formatMap = gcnew Dictionary<PixelFormat, ImageFormat>();

        formatMap->Add(PixelFormats::Gray8, ImageFormat::L_U8);
        formatMap->Add(PixelFormats::Bgr24, ImageFormat::BGR_U8);
        formatMap->Add(PixelFormats::Bgra32, ImageFormat::BGRA_U8);
        formatMap->Add(PixelFormats::Rgb24, ImageFormat::RGB_U8);

        _formatMap = gcnew ReadOnlyDictionary<PixelFormat, ImageFormat>(
            formatMap
            );

        auto typeMap = gcnew Dictionary<ImageFormat, int>();

        typeMap->Add(ImageFormat::L_U8, CV_8UC1);
        typeMap->Add(ImageFormat::RGB_U8, CV_8UC3);
        typeMap->Add(ImageFormat::BGR_U8, CV_8UC3);
        typeMap->Add(ImageFormat::BGRA_U8, CV_8UC4);

        _typeMap = gcnew ReadOnlyDictionary<ImageFormat, int>(typeMap);

        auto conversionCodeMap = gcnew Dictionary<uint32_t, int>();

        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::L_U8, ImageFormat::RGB_U8),
            cv::ColorConversionCodes::COLOR_GRAY2RGB
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::L_U8, ImageFormat::BGR_U8),
            cv::ColorConversionCodes::COLOR_GRAY2BGR
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::L_U8, ImageFormat::BGRA_U8),
            cv::ColorConversionCodes::COLOR_GRAY2BGRA
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::RGB_U8, ImageFormat::L_U8),
            cv::ColorConversionCodes::COLOR_RGB2GRAY
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::RGB_U8, ImageFormat::BGR_U8),
            cv::ColorConversionCodes::COLOR_RGB2BGR
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::RGB_U8, ImageFormat::BGRA_U8),
            cv::ColorConversionCodes::COLOR_RGB2BGRA
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::BGR_U8, ImageFormat::L_U8),
            cv::ColorConversionCodes::COLOR_BGR2GRAY
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::BGR_U8, ImageFormat::RGB_U8),
            cv::ColorConversionCodes::COLOR_BGR2RGB
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::BGR_U8, ImageFormat::BGRA_U8),
            cv::ColorConversionCodes::COLOR_BGR2BGRA
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::BGRA_U8, ImageFormat::L_U8),
            cv::ColorConversionCodes::COLOR_BGRA2GRAY
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::BGRA_U8, ImageFormat::RGB_U8),
            cv::ColorConversionCodes::COLOR_BGRA2RGB
        );
        conversionCodeMap->Add(
            IMAGE_FORMAT_PAIR(ImageFormat::BGRA_U8, ImageFormat::BGR_U8),
            cv::ColorConversionCodes::COLOR_BGRA2BGR
        );

        _conversionCodeMap = gcnew ReadOnlyDictionary<uint32_t, int>(
            conversionCodeMap
        );
    }

    ImageFormat Util::ToImageFormat(PixelFormat pixelFormat)
    {
        ImageFormat imageFormat;

        if (!_formatMap->TryGetValue(pixelFormat, imageFormat))
        {
            throw NotSupported(
                L"Unsupported pixel format. ({0})",
                pixelFormat
            );
        }

        return imageFormat;
    }

    PixelFormat Util::ToPixelFormat(ImageFormat imageFormat)
    {
        for each (KeyValuePair<PixelFormat, ImageFormat> entry in _formatMap)
        {
            if (entry.Value == imageFormat)
                return entry.Key;
        }

        throw NotSupported(
            L"Unsupported image format. ({0})",
            imageFormat
        );
    }

    int Util::ToType(ImageFormat imageFormat)
    {
        int type;

        if (!_typeMap->TryGetValue(imageFormat, type))
        {
            throw NotSupported(
                L"Unsupported image format. ({0})",
                imageFormat
            );
        }

        return type;
    }

	Pixmap ^Util::Cast(IPixmap ^pixmap)
	{
		return safe_cast<Pixmap^>(pixmap);
	}

    cv::Scalar Util::ToScalar(Color color, ImageFormat imageFormat)
    {
        cv::Scalar scalar;

        switch (imageFormat)
        {
        case ImageFormat::L_U8:
        case ImageFormat::RGB_U8:
            scalar[0] = (double)color.R;
            scalar[1] = (double)color.G;
            scalar[2] = (double)color.B;
            scalar[3] = (double)color.A;
            break;
        case ImageFormat::BGR_U8:
        case ImageFormat::BGRA_U8:
            scalar[0] = (double)color.B;
            scalar[1] = (double)color.G;
            scalar[2] = (double)color.R;
            scalar[3] = (double)color.A;
            break;
        default:
            throw NotSupported(
                L"Unsupported image format. ({0})",
                imageFormat
            );
            break;
        }

        return scalar;
    }

	Color Util::ToColor(const cv::Scalar &scalar, ImageFormat imageFormat)
	{
		Color color;

		switch (imageFormat)
		{
		case ImageFormat::L_U8:
			color = Color::FromRgb(
				(uint8_t)scalar[0],
				(uint8_t)scalar[0],
				(uint8_t)scalar[0]
			);
			break;
		case ImageFormat::BGR_U8:
			color = Color::FromRgb(
				(uint8_t)scalar[2],
				(uint8_t)scalar[1],
				(uint8_t)scalar[0]
			);
			break;
		case ImageFormat::BGRA_U8:
			color = Color::FromArgb(
				(uint8_t)scalar[3],
				(uint8_t)scalar[2],
				(uint8_t)scalar[1],
				(uint8_t)scalar[0]
			);
			break;
		default:
			throw NotSupported(
				L"Unsupported image format. ({0})",
				imageFormat
			);
			break;
		}

		return color;
	}

    int Util::GetConversionCode(ImageFormat src, ImageFormat dst)
    {
        int code;
        uint32_t key = IMAGE_FORMAT_PAIR(src, dst);

        if (!_conversionCodeMap->TryGetValue(key, code))
        {
            throw NotSupported(
                L"Unsupported conversion from {0} to {1}.",
                src,
                dst
            );
        }

        return code;
    }

    int Util::ToInt(KernelShape shape)
    {
        switch (shape)
        {
        case KernelShape::Rect:
            return cv::MorphShapes::MORPH_RECT;
        case KernelShape::Cross:
            return cv::MorphShapes::MORPH_CROSS;
        case KernelShape::Ellipse:
            return cv::MorphShapes::MORPH_ELLIPSE;
        default:
            throw NotSupported(
                String::Format(L"Unsupported kernel shape ({0}).", shape)
            );
        }
    }

	String ^Util::Marshal(const char *value)
	{
		return marshal_as<String^>(value);
	}

	std::string Util::Marshal(String ^value)
	{
		return marshal_as<std::string>(value);
	}

    Exception ^Util::NotSupported(String ^format, ... array<Object^> ^args)
    {
        return gcnew NotSupportedException(String::Format(format, args));
    }
}}}