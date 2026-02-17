#ifndef __MY_LIB_MATH_HEADER_H__
#define __MY_LIB_MATH_HEADER_H__

#include <numeric>
#include <numbers>
#include <bit>

#include <my-lib/std.h>
#include <my-lib/exception.h>

namespace Mylib
{
namespace Math
{

// ---------------------------------------------------

template <std::integral T>
constexpr bool is_power_of_two (const T value) noexcept
{
	return (value > 0) & ((value & (value - 1)) == 0);
}

template <std::integral T>
constexpr T log2_fast (const T value) noexcept
{
	using U = std::make_unsigned_t<T>;
	return static_cast<T>(std::bit_width(static_cast<U>(value)) - 1);
}

// If value is a power of two, returns the same value.
// Otherwise, returns the next power of two.

template <std::integral T>
constexpr T next_power_of_two_fast (const T value) noexcept
{
	using U = std::make_unsigned_t<T>;
	return static_cast<T>(std::bit_ceil(static_cast<U>(value)));
}

template <std::integral T>
constexpr T log2_safe (const T value)
{
	if (!is_power_of_two(value)) [[unlikely]]
		mylib_throw(InvalidNumberException);

	return log2_fast(value);
}

constexpr auto radians_to_degrees (const auto radians) noexcept -> decltype(radians)
{
	using Type = decltype(radians);
	return (radians * static_cast<Type>(180)) / std::numbers::pi_v<Type>;
}

constexpr auto degrees_to_radians (const auto degrees) noexcept -> decltype(degrees)
{
	using Type = decltype(degrees);
	return (degrees / static_cast<Type>(180)) * std::numbers::pi_v<Type>;
}

template <typename Tout>
constexpr Tout round_to_nearest (const auto v) noexcept
{
	return static_cast<Tout>(v + static_cast<decltype(v)>(0.5));
}

// ---------------------------------------------------

} // end namespace Math
} // end namespace Mylib

#endif