#include "Converters.h"

#include <array>
#include <charconv>
#include <cctype>

namespace encoding
{
	namespace converters
	{
		std::string convertUTF16_ASCII(std::wstring_view text) // Every non ascii (0-127) character will be casted to 128
		{
			std::string converted{};
			for (auto it = text.begin(); it != text.end(); it++)
			{
				if (*it >= 0xD800 && *it <= 0xDFFF)
				{
					it++;
					if (it == text.end())
					{
						throw ConvertionError{ "Invalid UTF16 text length" };
					}
					if (*it < 0xDC00 || *it > 0xDFFF)
					{
						throw ConvertionError{ "Invalid UTF16 encoding" };
					}

					converted += static_cast<unsigned char>(128);
				}
				else
				{
					if (*it < 128)
					{
						converted += static_cast<char>(*it);
					}
					else
					{
						converted += static_cast<unsigned char>(128);
					}
				}
			}

			return converted;
		}

		std::wstring convertASCII_UTF16(std::string_view text)
		{
			std::wstring converted{};
			std::transform(text.begin(), text.end(), std::back_insert_iterator<std::wstring>(converted),
				[](char x) { if (static_cast<unsigned char>(x) > 127) { throw ConvertionError{ "Invalid ASCII encoding" }; } return x; }
			);
			return converted;
		}

		std::string convertURLEncode_UTF8(std::string_view text)
		{
			std::string converted{};
			for (auto it = text.begin(); it != text.end(); it++)
			{
				switch (*it)
				{
				case '+':
					converted += ' ';
					break;

				case ' ':
					throw ConvertionError{ "Invalid URLEncode encoding" };
					break;

				case '%':
				{
					std::array<char, 2> hex{};

					it++;
					if (it == text.end()) { throw ConvertionError{ "Invalid URLEncode encoding" }; }
					hex.at(0) = *it;
					it++;
					if (it == text.end()) { throw ConvertionError{ "Invalid URLEncode encoding" }; }
					hex.at(1) = *it;

					char character;
					auto[ptr, ec] = std::from_chars(hex.data(), hex.data() + 2, character, 16);

					if (ptr != hex.data() + 2) { throw ConvertionError{ "Invalid URLEncode encoding" }; }

					converted += character;

				}
				break;

				default:
					converted += *it;
					break;
				}
			}

			return converted;
		}

		void convertURLEncode_UTF8(std::string && text)
		{
			for (auto it = text.begin(); it != text.end(); it++)
			{
				switch (*it)
				{
				case '+':
					*it = ' ';
					break;

				case ' ':
					throw ConvertionError{ "Invalid URLEncode encoding" };
					break;

				case '%':
				{
					std::array<char, 2> hex{};

					it = text.erase(it);
					if (it == text.end()) { throw ConvertionError{ "Invalid URLEncode encoding" }; }
					hex.at(0) = *it;
					it = text.erase(it);
					if (it == text.end()) { throw ConvertionError{ "Invalid URLEncode encoding" }; }
					hex.at(1) = *it;

					char character;
					auto[ptr, ec] = std::from_chars(hex.data(), hex.data() + 2, character, 16);

					if (ptr != hex.data() + 2) { throw ConvertionError{ "Invalid URLEncode encoding" }; }

					*it = character;

				}
				break;

				default:
					break;
				}
			}
		}

		std::string convertUTF8_URLEncode(std::string_view text)
		{
			std::string converted{};
			for (auto it = text.begin(); it != text.end(); it++)
			{
				if (std::isalnum(*it))
				{
					converted += *it;
				}
				else
				{
					std::string hex{ "FF" };
					auto[ptr, ec] = std::to_chars(hex.data(), hex.data() + 2, static_cast<unsigned char>(*it), 16);

					if (ptr != hex.data() + 2) { throw ConvertionError{ "Invalid ASCII encoding" }; }

					converted += '%';
					converted += hex;
				}
			}

			return converted;
		}

		void convertUTF8_URLEncode(std::string && text)
		{
			for (std::size_t i = 0; i < text.size(); i++)
			{
				if (!std::isalnum(text.at(i)))
				{
					std::string hex{ "FF" };
					auto[ptr, ec] = std::to_chars(hex.data(), hex.data() + 2, static_cast<unsigned char>(text.at(i)), 16);

					if (ptr != hex.data() + 2) { throw ConvertionError{ "Invalid ASCII encoding" }; }

					text.replace(i, 1, '%' + hex);
					i += 2;
				}
			}
		}

		std::pair<char, std::array<char16_t, 2>> characterToUTF16(char32_t character)
		{
			std::array<char16_t, 2> character_utf16;
			unsigned char character_lenght;

			if (character >= 0xD800 && character <= 0xDFFF)
			{
				throw ConvertionError{ "Invalid UTF32 encoding" };
			}
			else if (character < 0x10000)
			{
				character_lenght = 1;
				character_utf16.at(0) = static_cast<char16_t>(character);
			}
			else if (character < 0x110000)
			{
				character_lenght = 2;

				character -= 0x10000;
				character_utf16.at(0) = static_cast<char16_t>((character >> 10u) + 0xD800u);
				character_utf16.at(1) = static_cast<char16_t>((character & 0x3FFu) + 0xDC00u);
			}
			else
			{
				throw ConvertionError{ "Invalid UTF32 encoding" };
			}

			return { character_lenght, character_utf16 };
		}

		char32_t characterFromUTF16(unsigned char character_lenght, const std::array<char16_t, 2> & character_utf16)
		{
			char32_t characterUTF32 = 0;

			if (character_lenght == 1)
			{
				characterUTF32 = character_utf16.at(0);
			}
			else if (character_lenght == 2)
			{
				if (!(
					(character_utf16.at(0) >= 0xD800 && character_utf16.at(0) <= 0xDBFF) &&
					(character_utf16.at(1) >= 0xDC00 && character_utf16.at(1) <= 0xDFFF)
					))
				{
					throw ConvertionError{ "Invalid UTF16 encoding" };
				}

				characterUTF32 = ((character_utf16.at(0) - 0xD800) << 10) + (character_utf16.at(1) - 0xDC00) + 0x10000;
			}

			return characterUTF32;
		}

		std::pair<char, std::array<unsigned char, 4>> characterToUTF8(char32_t character)
		{
			std::array<unsigned char, 4> character_utf8;
			unsigned char character_lenght;

			if (character < 0x80)
			{
				character_lenght = 1;
				character_utf8.at(0) = static_cast<unsigned char>(character);
			}
			else if (character < 0x800)
			{
				character_lenght = 2;
				character_utf8.at(0) = 0xC0 | static_cast<unsigned char>((character >> 6) & 0x1F);
				character_utf8.at(1) = 0x80 | static_cast<unsigned char>(character & 0x3F);
			}
			else if (character < 0x10000)
			{
				character_lenght = 3;
				character_utf8.at(0) = 0xE0 | static_cast<unsigned char>((character >> 12) & 0x0F);
				character_utf8.at(1) = 0x80 | static_cast<unsigned char>((character >> 6) & 0x3F);
				character_utf8.at(2) = 0x80 | static_cast<unsigned char>(character & 0x3F);
			}
			else if (character < 0x110000)
			{
				character_lenght = 4;
				character_utf8.at(0) = 0xF0 | static_cast<unsigned char>((character >> 18) & 0x07);
				character_utf8.at(1) = 0x80 | static_cast<unsigned char>((character >> 12) & 0x3F);
				character_utf8.at(2) = 0x80 | static_cast<unsigned char>((character >> 6) & 0x3F);
				character_utf8.at(3) = 0x80 | static_cast<unsigned char>(character & 0x3F);
			}
			else
			{
				throw ConvertionError{ "Invalid UTF32 encoding" };
			}

			return { character_lenght, character_utf8 };
		}

		char32_t characterFromUTF8(unsigned char character_lenght, const std::array<unsigned char, 4> & character_utf8)
		{
			char32_t characterUTF32 = 0;

			if (character_lenght == 1)
			{
				characterUTF32 = character_utf8.at(0);
			}
			else if (character_lenght == 2)
			{
				if (
					(character_utf8.at(1) & 0xC0) ^ 0x80
					)
				{
					throw ConvertionError{ "Invalid UTF8 encoding" };
				}

				characterUTF32 = (character_utf8.at(1) & 0x3Fu) | ((character_utf8.at(0) & 0x1Fu) << 6u);

				if (characterUTF32 < 0x80) { throw ConvertionError{ "Invalid UTF8 encoding" }; }
			}
			else if (character_lenght == 3)
			{
				if (
					(character_utf8.at(1) & 0xC0) ^ 0x80 ||
					(character_utf8.at(2) & 0xC0) ^ 0x80
					)
				{
					throw ConvertionError{ "Invalid UTF8 encoding" };
				}

				characterUTF32 = (character_utf8.at(2) & 0x3Fu) | ((character_utf8.at(1) & 0x3Fu) << 6u) | ((character_utf8.at(0) & 0x0Fu) << 12u);

				if (characterUTF32 < 0x800) { throw ConvertionError{ "Invalid UTF8 encoding" }; }
			}
			else
			{
				if (
					(character_utf8.at(0) & 0xF8) ^ 0xF0 ||
					(character_utf8.at(1) & 0xC0) ^ 0x80 ||
					(character_utf8.at(2) & 0xC0) ^ 0x80 ||
					(character_utf8.at(3) & 0xC0) ^ 0x80
					)
				{
					throw ConvertionError{ "Invalid UTF8 encoding" };
				}

				characterUTF32 = (character_utf8.at(3) & 0x3Fu) | ((character_utf8.at(2) & 0x3Fu) << 6) | ((character_utf8.at(1) & 0x3Fu) << 12) | ((character_utf8.at(0) & 0x07u) << 18);

				if (characterUTF32 < 0x10000 || characterUTF32 > 0x10FFFF) { throw ConvertionError{ "Invalid UTF8 encoding" };; }
			}

			if (characterUTF32 >= 0xD800 && characterUTF32 <= 0xDFFF)
			{
				throw ConvertionError{ "Invalid UTF8 encoding" };
			}

			return characterUTF32;
		}

		std::wstring convertUTF8_UTF16(std::string_view text)
		{
			std::wstring converted{};

			for (auto it = text.begin(); it != text.end();)
			{
				std::array<unsigned char, 4> character_utf8;
				unsigned char character_lenght;
				unsigned char first_byte = *it;

				if (first_byte < 0x80)
				{
					character_lenght = 1;
				}
				else if (first_byte < 0xE0)
				{
					character_lenght = 2;
				}
				else if (first_byte < 0xF0)
				{
					character_lenght = 3;
				}
				else if (first_byte < 0xF5)
				{
					character_lenght = 4;
				}
				else
				{
					throw ConvertionError{ "Invalid UTF8 encoding" };
				}

				for (std::size_t i = 0; i < character_lenght; i++)
				{
					if (it == text.end())
					{
						throw ConvertionError{ "Invalid UTF8 encoding" };
					}

					character_utf8.at(i) = *it;
					++it;
				}

				auto[size, character_UTF16] = characterToUTF16(characterFromUTF8(character_lenght, character_utf8));

				for (char i = 0; i < size; i++)
				{
					converted += character_UTF16.at(i);
				}
			}

			return converted;
		}

		std::string convertUTF16_UTF8(std::wstring_view text)
		{
			std::string converted{};

			for (auto it = text.begin(); it != text.end();)
			{
				std::array<char16_t, 2> character_utf16;
				unsigned char character_lenght;
				char16_t first_byte = *it;

				if (first_byte < 0xD800 || first_byte > 0xDFFF)
				{
					character_lenght = 1;
				}
				else
				{
					character_lenght = 2;
				}

				for (std::size_t i = 0; i < character_lenght; i++)
				{
					if (it == text.end())
					{
						throw ConvertionError{ "Invalid UTF16 encoding" };
					}

					character_utf16.at(i) = *it;
					++it;
				}

				auto[size, character_UTF8] = characterToUTF8(characterFromUTF16(character_lenght, character_utf16));

				for (char i = 0; i < size; i++)
				{
					converted += character_UTF8.at(i);
				}
			}

			return converted;
		}

	}
}
