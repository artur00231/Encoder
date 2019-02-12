
#include "Encoder.h"

#include <iostream>


int main()
{
	encoding::makeEncoder<encoding::UTF16, encoding::UTF8> encoder{};

	std::wstring x{ (char16_t)0x0024, (char16_t)0x00A2, (char16_t)0x0939, (char16_t)0x20AC, (char16_t)0xD800, (char16_t)0xDF48 };
	auto text = encoder.convert(x);

	std::cout << std::hex << std::uppercase;

	for (auto && y : text)
	{
		std::cout << static_cast<unsigned int>(static_cast<unsigned char>(y)) << '\n';
	}

	encoding::makeEncoder<encoding::URLEncode, encoding::ASCII, false> encoder2{};
	std::string s{ "ala" };
	std::cout << encoder2.convert(s);

	return 0;
}