#ifndef ENCODER_H
#define ENCODER_H

#include "BaseEncoders.h"


#include <utility>
#include <array>
#include <cassert>

namespace encoding
{
	namespace helpers
	{
		template<typename T, typename = bool>
		struct isEncoder : std::false_type {};

		template<typename T>
		struct isEncoder<T, decltype(std::is_same_v<typename T::output_type, decltype(std::declval<T>().convert(std::declval<typename T::input_type>()))> && T::is_base_encoder::value && T::is_lossless::value)> : std::true_type {};

	}

	template<typename T>
	constexpr inline bool isEncoder() noexcept { return helpers::isEncoder<T>::value; }

	template<typename T>
	constexpr inline bool isBaseEncoder() noexcept {
		if constexpr (isEncoder<T>())
			return T::is_base_encoder::value;
		else
			return false;
	}

	template<typename T>
	constexpr inline bool isCombinedEncoder() noexcept {
		if constexpr (isEncoder<T>())
			return !T::is_base_encoder::value;
		else
			return false;
	}

	template<typename T>
	constexpr inline bool isLosslessEncoder() noexcept {
		if constexpr (isEncoder<T>())
			return T::is_lossless::value;
		else
			return false;
	}

	template<typename T, typename U>
	constexpr inline bool canBeCombinedEncoder() noexcept {
		if constexpr (isEncoder<T>() && isEncoder<U>())
			return std::is_same_v<typename T::output_type, typename U::input_type> || std::is_convertible_v<typename T::output_type, typename U::input_type>;
		else
			return false;
	}

	template<typename T, typename U, std::enable_if_t<canBeCombinedEncoder<T, U>(), int> = 0>
	class CombinedEncoder
	{
	public:
		using input_type = typename T::input_type;
		using output_type = typename U::output_type;

		using is_base_encoder = std::false_type;
		using is_lossless = std::conditional_t<T::is_lossless::value && U::is_lossless::value, std::true_type, std::false_type>;

		output_type convert(input_type text) const
		{
			U u; T t;
			return u.convert(t.convert(text));
		}
	};

	namespace helpers
	{
		template<typename T, typename U, typename = void>
		struct existsBaseEncoder : std::false_type {};

		template<typename T, typename U>
		struct existsBaseEncoder<T, U, std::void_t<typename Encoder<T, U>::input_type>> : std::true_type {};
	}

	template<typename T, typename U>
	constexpr inline bool existsBaseEncoder() noexcept { return helpers::existsBaseEncoder<T, U>::value; }

	namespace helpers
	{
		template<std::size_t parent, std::size_t child>
		inline constexpr auto generateConnectionsMatrixRow(std::array<std::pair<bool, bool>, encoding_count * encoding_count> connections)->std::array<std::pair<bool, bool>, encoding_count * encoding_count>
		{
			if constexpr (child == encoding_count)
			{
				return connections;
			}
			else
			{
				if constexpr (existsBaseEncoder<encoding_code<parent>, encoding_code<child>>())
				{
					connections.at(parent * encoding_count + child).first = true;
					connections.at(parent * encoding_count + child).second = isLosslessEncoder<Encoder< encoding_code<parent>, encoding_code<child>>>();
				}
				else
				{
					connections.at(parent * encoding_count + child).first = false;
					connections.at(parent * encoding_count + child).second = false;
				}

				return generateConnectionsMatrixRow<parent, child + 1>(connections);
			}
		}

		template<std::size_t encoding_code>
		inline constexpr auto generateConnectionsMatrixColumn(std::array<std::pair<bool, bool>, encoding_count * encoding_count> connections = {})->std::array<std::pair<bool, bool>, encoding_count * encoding_count>
		{
			if constexpr (encoding_code == encoding_count)
			{
				return connections;
			}
			else
			{
				return generateConnectionsMatrixColumn<encoding_code + 1>(generateConnectionsMatrixRow<encoding_code, 0>(connections));
			}
		}

		constexpr static std::array<std::pair<bool, bool>, encoding_count * encoding_count> connections{ generateConnectionsMatrixColumn<0>() };

		inline constexpr std::array<std::size_t, encoding_count + 1> generatePath(std::size_t begin, std::size_t end, bool lossless)
		{
			constexpr std::size_t npos = -1;

			std::array<std::size_t, encoding_count + 1> queue{ begin };
			std::size_t p = 0, q = 1, current_encoding = begin;
			std::array<std::size_t, encoding_count> parents{};
			for (auto & x : parents) //fill the array with npos
				x = npos;

			while (p != q)
			{
				current_encoding = queue.at(p);
				++p;

				for (std::size_t i = 0; i < encoding_count; ++i)
				{
					if (auto connection = connections.at(current_encoding * encoding_count + i); connection.first && parents.at(i) == npos && (connection.second || !lossless))
					{
						parents.at(i) = current_encoding;
						queue.at(q) = i;
						++q;
					}
				}
			}

			parents.at(begin) = npos;

			assert(parents.at(end) != npos); //Path does't exist

			std::array<std::size_t, encoding_count + 1> rpath{}; //the reverse path
			for (auto & x : rpath) //fill the array with npos
				x = npos;
			rpath.at(0) = npos;
			rpath.at(1) = end;

			current_encoding = end;
			q = 2;
			while (parents.at(current_encoding) != npos)
			{
				current_encoding = rpath.at(q) = parents.at(current_encoding);
				q++;
			}

			std::array<std::size_t, encoding_count + 1> path{};
			for (auto & x : path) //fill the array with npos
				x = npos;

			for (std::size_t i = encoding_count, j = 0; i >= 0 + 1; i--) //reverse the rpath
			{															 //don't copy npos
				if (rpath.at(i) != npos)
				{
					path.at(j) = rpath.at(i);
					j++;
				}
			}

			return path;
		}

		template<typename T, typename U, bool LOSSLESS>
		static constexpr std::array<std::size_t, encoding_count + 1> path{ generatePath(T::value, U::value, LOSSLESS) };

		template<typename T, typename U>
		constexpr inline bool existsLosslessBaseEncoder()
		{
			if constexpr (existsBaseEncoder<T, U>())
			{
				return Encoder<T, U>::is_lossless();
			}
			else
			{
				return false;
			}
		}

		template<typename T, typename U, bool LOSSLESS, std::enable_if_t<existsBaseEncoder<T, U>() && (!LOSSLESS || existsLosslessBaseEncoder<T, U>()), int> = 0>
		constexpr inline auto makeEncoder() noexcept->Encoder<T, U>;

		template<std::size_t N, const std::array<std::size_t, N> & arr, std::size_t INDEX>
		auto makeCombinedEncoder(typename std::enable_if_t<arr[INDEX + 2] == -1, bool> = true)->Encoder<encoding_code<arr[INDEX]>, encoding_code<arr[INDEX + 1]>>;
		
		template<std::size_t N, const std::array<std::size_t, N> & arr, std::size_t INDEX>
		auto makeCombinedEncoder(typename std::enable_if_t<arr[INDEX + 2] != -1, bool> = true)
		{
			return decltype(CombinedEncoder<
				Encoder<encoding_code<arr[INDEX]>, encoding_code<arr[INDEX + 1]>>,
				decltype(makeCombinedEncoder<N, arr, INDEX + 1>())>()
				) {};
		}

		template<typename T, typename U, bool LOSSLESS, std::enable_if_t<!(existsBaseEncoder<T, U>() && (!LOSSLESS || existsLosslessBaseEncoder<T, U>())), int> = 0>
		constexpr inline auto makeEncoder() noexcept->decltype(makeCombinedEncoder<encoding_count + 1, path<T, U, LOSSLESS>, 0>());

	}

	template<typename T, typename U, bool LOSSLESS = true>
	using makeEncoder = decltype(helpers::makeEncoder<T, U, LOSSLESS>());
}

#endif // !ENCODER_H
