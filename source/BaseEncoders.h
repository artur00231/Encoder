#ifndef BASE_ENCODER_H
#define BASE_ENCODER_H

#include "Encoding.h"
#include "Converters.h"

#include <string>

namespace encoding
{
	template<typename T, typename U>
	class Encoder
	{
	};

	//The encoder have to have the default constructor
	//The minimal encoder should contain the following elements
	/*template<>
	class Encoder<T, U>
	{
	public:
		using input_type (e.g. std::string_view, const std::string &)
		using output_type

		using is_base_encoder = std::true_type;
		using is_lossless = std::true_type | std::false_type;

		output_type convert(input_type) const;

	};*/


	template<>
	class Encoder<UTF8, UTF16>
	{
	public:
		using input_type = std::string_view;
		using output_type = std::wstring;

		using is_base_encoder = std::true_type;
		using is_lossless = std::true_type;

		output_type convert(input_type text) const
		{
			return converters::convertUTF8_UTF16(text);
		}
	};

	template<>
	class Encoder<UTF16, UTF8>
	{
	public:
		using input_type = std::wstring_view;
		using output_type = std::string;

		using is_base_encoder = std::true_type;
		using is_lossless = std::true_type;

		output_type convert(input_type text) const
		{
			return converters::convertUTF16_UTF8(text);
		}

	};

	template<>
	class Encoder<URLEncode, UTF8>
	{
	public:
		using input_type = std::string_view;
		using output_type = std::string;

		using is_base_encoder = std::true_type;
		using is_lossless = std::true_type;

		output_type convert(input_type text) const
		{
			return converters::convertURLEncode_UTF8(text);
		}

		void convert(std::string && text) const
		{
			converters::convertURLEncode_UTF8(std::move(text));
		}
	};

	template<>
	class Encoder<UTF8, URLEncode>
	{
	public:
		using input_type = std::string_view;
		using output_type = std::string;

		using is_base_encoder = std::true_type;
		using is_lossless = std::true_type;

		output_type convert(input_type text) const
		{
			return converters::convertUTF8_URLEncode(text);
		}

		void convert(std::string && text) const
		{
			converters::convertUTF8_URLEncode(std::move(text));
		}
	};

	template<>
	class Encoder<UTF16, ASCII>
	{
	public:
		using input_type = std::wstring_view;
		using output_type = std::string;

		using is_base_encoder = std::true_type;
		using is_lossless = std::false_type;

		output_type convert(input_type text) const
		{
			return converters::convertUTF16_ASCII(text);
		}
	};

	template<>
	class Encoder<ASCII, UTF16>
	{
	public:
		using input_type = std::string_view;
		using output_type = std::wstring;

		using is_base_encoder = std::true_type;
		using is_lossless = std::true_type;

		output_type convert(input_type text) const
		{
			return converters::convertASCII_UTF16(text);
		}
	};
}

#endif // !BASE_ENCODER_H
