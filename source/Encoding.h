#ifndef ENCODING_H
#define ENCODING_H

#include <type_traits>

namespace encoding
{
	template<std::size_t U>
	using encoding_code = std::integral_constant<std::size_t, U>;

	constexpr std::size_t encoding_count = 4;	// Number of available encodings
												// Encodings must have integral_constant values from 0 to encoding_count-1

	using UTF8 = encoding_code<0>;
	using UTF16 = encoding_code<1>;
	using URLEncode = encoding_code<2>;
	using ASCII = encoding_code<3>;
}


#endif // !ENCODING_H