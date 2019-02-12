#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <exception>
#include <string>
#include <algorithm>
#include <iterator>

namespace encoding
{
	class ConvertionError : public std::exception
	{
	public:
		ConvertionError(const char *message) : message{ message } {};
		ConvertionError(const std::string & message) : message{ message } {};
		~ConvertionError() noexcept {};

		const char* what() const noexcept override
		{
			return message.c_str();
		}
	private:
		std::string message;
	};

	namespace converters
	{
		std::string convertUTF16_ASCII(std::wstring_view text); // Every non ascii (0-127) character will be casted to 128

		std::wstring convertASCII_UTF16(std::string_view text);

		std::string convertURLEncode_UTF8(std::string_view text);
		void convertURLEncode_UTF8(std::string && text);

		std::string convertUTF8_URLEncode(std::string_view text);
		void convertUTF8_URLEncode(std::string && text);
		
		std::wstring convertUTF8_UTF16(std::string_view text);

		std::string convertUTF16_UTF8(std::wstring_view text);

	}
}

#endif // !CONVERTERS_H
