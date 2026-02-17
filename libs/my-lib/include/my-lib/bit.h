#ifndef __MY_LIB_BIT_HEADER_H__
#define __MY_LIB_BIT_HEADER_H__

#include <concepts>
#include <type_traits>
#include <ostream>
#include <limits>

#include <cstdint>

#include <my-lib/std.h>

namespace Mylib
{

#ifdef MYLIB_BUILD_OPERATION
#error nooooooooooo
#endif

// ---------------------------------------------------

struct BitField
{
	uint16_t bpos;     // bit start position
	uint16_t blength;  // bit length
};

// ---------------------------------------------------

template <std::unsigned_integral T>
constexpr T make_bit_mask (const std::size_t blength) noexcept
{
	return (blength < std::numeric_limits<T>::digits) ?
		((static_cast<T>(1) << blength) - 1) :
		std::numeric_limits<T>::max();
}

// ---------------------------------------------------

template <std::unsigned_integral T>
constexpr T get_bits (const T v, const std::size_t bpos, const std::size_t blength) noexcept
{
	const T mask = make_bit_mask<T>(blength);
	return (v >> bpos) & mask;
}

template <std::signed_integral T>
constexpr T get_bits (const T v, const std::size_t bpos, const std::size_t blength) noexcept
{
	using U = std::make_unsigned_t<T>;
	return static_cast<T>(get_bits(static_cast<U>(v), bpos, blength));
}

template <std::integral T>
constexpr T get_bits (const T v, const BitField field) noexcept
{
	return get_bits(v, field.bpos, field.blength);
}

template <std::integral T>
constexpr T get_bits (const T v, const auto bpos, const auto blength) noexcept
	requires std::is_enum_v<decltype(bpos)> && std::is_enum_v<decltype(blength)>
{
	return get_bits(v, std::to_underlying(bpos), std::to_underlying(blength));
}

// ---------------------------------------------------

template <std::unsigned_integral T>
constexpr T set_bits (const T src, const std::size_t bpos, const std::size_t blength, const std::integral auto value) noexcept
{
	const T mask = make_bit_mask<T>(blength);
	const T shifted_mask = mask << bpos;
	const T safe_value = static_cast<T>(value) & mask;

	return (src & ~shifted_mask) | (safe_value << bpos);
}

template <std::signed_integral T>
constexpr T set_bits (const T src, const std::size_t bpos, const std::size_t blength, const std::integral auto value) noexcept
{
	using U = std::make_unsigned_t<T>;
	return static_cast<T>(set_bits(static_cast<U>(src), bpos, blength, value));
}

template <std::integral T>
constexpr T set_bits (const T src, const BitField field, const std::integral auto value) noexcept
{
	return set_bits(src, field.bpos, field.blength, value);
}

template <std::integral T>
constexpr T set_bits (const T src, const auto bpos, const auto blength, const std::integral auto value) noexcept
	requires std::is_enum_v<decltype(bpos)> && std::is_enum_v<decltype(blength)>
{
	return set_bits(src, std::to_underlying(bpos), std::to_underlying(blength), value);
}

// ---------------------------------------------------

template <std::size_t storage_nbits>
class BitSetStorage__;

template <>
class BitSetStorage__<8>
{
public:
	using Type = uint8_t;
protected:
	Type storage__;
};

template <>
class BitSetStorage__<16>
{
public:
	using Type = uint16_t;
protected:
	Type storage__;
};

template <>
class BitSetStorage__<32>
{
public:
	using Type = uint32_t;
protected:
	Type storage__;
};

template <>
class BitSetStorage__<64>
{
public:
	using Type = uint64_t;
protected:
	Type storage__;
};

// ---------------------------------------------------

template <typename ParentType>
class BitSetStorage_ : public ParentType
{
protected:
	using Type = typename ParentType::Type;

	Type& storage () noexcept
	{
		return this->storage__;
	}

	Type storage () const noexcept
	{
		return this->storage__;
	}
};

// ---------------------------------------------------

consteval std::size_t calc_bit_set_storage_nbits__ (const std::size_t nbits) noexcept
{
	if (nbits <= 8)
		return 8;
	else if (nbits <= 16)
		return 16;
	else if (nbits <= 32)
		return 32;
	else if (nbits <= 64)
		return 64;
	else
		return 0;
}

// ---------------------------------------------------

/*
	BitSet__ is based on std::bitset.
	Differences:
		- Only works for up tp 64 bits, but it is faster.
		- Allows extraction and set of range of bits.
*/

template <typename ParentType, std::size_t nbits>
class BitSet__ : public ParentType
{
public:
	using Type = typename ParentType::Type;

	static consteval std::size_t get_storage_nbits__ () noexcept
	{
		return sizeof(Type) * 8;
	}

	static consteval std::size_t size__ () noexcept
	{
		return nbits;
	}

private:
	static constexpr Type safety_mask = (nbits == get_storage_nbits__()) ?
		std::numeric_limits<Type>::max() :
		(static_cast<Type>(1) << nbits) - 1;

	static consteval bool must_apply_safety_mask () noexcept
	{
		return (nbits < get_storage_nbits__());
	}

	constexpr void ensure_safety_mask () noexcept
	{
		if constexpr (must_apply_safety_mask())
			this->storage() &= safety_mask;
	}

public:
	constexpr std::size_t get_storage_nbits () const noexcept
	{
		return get_storage_nbits__();
	}

	constexpr std::size_t size () const noexcept
	{
		return size__();
	}

	// --------------------------

	constexpr BitSet__ () noexcept
	{
		this->storage() = 0;
	}

	constexpr BitSet__ (const BitSet__& other) noexcept
	{
		this->storage() = other.storage();
		this->ensure_safety_mask();
	}

	constexpr BitSet__ (const std::integral auto v) noexcept
	{
		this->storage() = static_cast<Type>(v);
		this->ensure_safety_mask();
	}

	// --------------------------

	constexpr BitSet__& operator= (const BitSet__& other) noexcept
	{
		this->storage() = other.storage();
		this->ensure_safety_mask();
		return *this;
	}

	constexpr BitSet__& operator= (const std::integral auto v) noexcept
	{
		this->storage() = static_cast<Type>(v);
		this->ensure_safety_mask();
		return *this;
	}

	// --------------------------

	constexpr Type to_underlying () const noexcept
	{
		return this->storage();
	}

	constexpr Type operator* () const noexcept
	{
		return this->storage();
	}

	// -------------------------
	
	constexpr Type operator[] (const std::size_t pos) const noexcept
	{
		return (this->storage() >> pos) & 0x01;
	}

	constexpr Type operator[] (const std::size_t pos) noexcept
	{
		return (this->storage() >> pos) & 0x01;
	}

	template <typename Tenum>
	requires std::is_enum_v<Tenum>
	constexpr Type operator[] (const Tenum pos) const noexcept
	{
		return (*this)[std::to_underlying(pos)];
	}

	template <typename Tenum>
	requires std::is_enum_v<Tenum>
	constexpr Type operator[] (const Tenum pos) noexcept
	{
		return (*this)[std::to_underlying(pos)];
	}

	// --------------------------

	constexpr Type operator[] (const BitField field) const noexcept
	{
		return get_bits(this->storage(), field.bpos, field.blength);
	}

	constexpr Type operator[] (const BitField field) noexcept
	{
		return get_bits(this->storage(), field.bpos, field.blength);
	}

	// --------------------------

	constexpr Type operator[] (const std::size_t pos, const std::size_t length) const noexcept
	{
		return get_bits(this->storage(), pos, length);
	}

	constexpr Type operator[] (const std::size_t pos, const std::size_t length) noexcept
	{
		return get_bits(this->storage(), pos, length);
	}

	// --------------------------

	constexpr BitSet__ operator() (const std::size_t pos, const std::size_t length) const noexcept
	{
		return BitSet__(get_bits(this->storage(), pos, length));
	}

	template <typename TenumA, typename TenumB>
	requires std::is_enum_v<TenumA> && std::is_enum_v<TenumB>
	constexpr BitSet__ operator() (const TenumA pos, const TenumB length) const noexcept
	{
		return (*this)(std::to_underlying(pos), std::to_underlying(length));
	}

	constexpr BitSet__ operator() (const BitField field) const noexcept
	{
		return (*this)(field.bpos, field.blength);
	}

	// --------------------------

	constexpr Type get (const std::size_t pos, const std::size_t length) const noexcept
	{
		return get_bits(this->storage(), pos, length);
	}

	template <typename TenumA, typename TenumB>
	requires std::is_enum_v<TenumA> && std::is_enum_v<TenumB>
	constexpr Type get (const TenumA pos, const TenumB length) const noexcept
	{
		return this->get(std::to_underlying(pos), std::to_underlying(length));
	}

	constexpr Type get (const BitField field) const noexcept
	{
		return this->get(field.bpos, field.blength);
	}

	// --------------------------

	constexpr Type get (const std::size_t pos) const noexcept
	{
		return get_bits(this->storage(), pos, 1);
	}

	template <typename Tenum>
	requires std::is_enum_v<Tenum>
	constexpr Type get (const Tenum pos) const noexcept
	{
		return this->get(std::to_underlying(pos));
	}

	// --------------------------

	constexpr BitSet__& set (const std::size_t pos, const std::size_t length, const std::integral auto v) noexcept
	{
		this->storage() = set_bits(this->storage(), pos, length, v);
		this->ensure_safety_mask();
		return *this;
	}

	constexpr BitSet__& set (const BitField field, const std::integral auto v) noexcept
	{
		this->storage() = set_bits(this->storage(), field.bpos, field.blength, v);
		this->ensure_safety_mask();
		return *this;
	}

	constexpr BitSet__& set (const Enum auto pos, const Enum auto length, const std::integral auto v) noexcept
	{
		this->storage() = set_bits(this->storage(), std::to_underlying(pos), std::to_underlying(length), v);
		this->ensure_safety_mask();
		return *this;
	}

	// --------------------------

	constexpr BitSet__& set (const std::size_t pos, const std::integral auto v) noexcept
	{
		this->storage() = set_bits(this->storage(), pos, 1, v);
		this->ensure_safety_mask();
		return *this;
	}

	constexpr BitSet__& set (const Enum auto pos, const std::integral auto v) noexcept
	{
		this->storage() = set_bits(this->storage(), std::to_underlying(pos), 1, v);
		this->ensure_safety_mask();
		return *this;
	}

	// --------------------------

	#ifdef MYLIB_BUILD_OPERATION
		#undef MYLIB_BUILD_OPERATION
	#endif

	#define MYLIB_BUILD_OPERATION(OP) \
		constexpr BitSet__& operator OP (const BitSet__ other) noexcept \
		{ \
			this->storage() OP other.storage(); \
			this->ensure_safety_mask(); \
			return *this; \
		} \
		constexpr BitSet__& operator OP (const std::integral auto v) noexcept \
		{ \
			this->storage() OP static_cast<Type>(v); \
			this->ensure_safety_mask(); \
			return *this; \
		}
	
	MYLIB_BUILD_OPERATION( &= )
	MYLIB_BUILD_OPERATION( |= )
	MYLIB_BUILD_OPERATION( ^= )

	#undef MYLIB_BUILD_OPERATION

	// --------------------------

	#define MYLIB_BUILD_OPERATION(OP) \
		constexpr BitSet__& operator OP (const std::integral auto v) noexcept \
		{ \
			this->storage() OP v; \
			this->ensure_safety_mask(); \
			return *this; \
		}
	
	MYLIB_BUILD_OPERATION( <<= )
	MYLIB_BUILD_OPERATION( >>= )

	#undef MYLIB_BUILD_OPERATION

	// --------------------------

	constexpr BitSet__ operator~ () const noexcept
	{
		return BitSet__(~this->storage());
	}

	// --------------------------

	constexpr bool all () const noexcept
	{
		return (this->storage() == safety_mask);
	}

	constexpr bool any () const noexcept
	{
		return (this->storage() != 0);
	}

	constexpr bool none () const noexcept
	{
		return (this->storage() == 0);
	}

	constexpr std::size_t count () const noexcept
	{
		std::size_t c = 0;
		Type v = this->storage();
		while (v) {
			c += (v & 0x01);
			v >>= 1;
		}
		return c;
	}

	// --------------------------

	constexpr BitSet__& reset () noexcept
	{
		this->storage() = 0;
		return *this;
	}

	constexpr BitSet__& reset (const std::size_t pos, const std::size_t length) noexcept
	{
		this->storage() = set_bits(this->storage(), pos, length, 0);
		return *this;
	}

	constexpr BitSet__& reset (const std::size_t pos) noexcept
	{
		return this->reset(pos, 1);
	}

	constexpr BitSet__& reset (const BitField field) noexcept
	{
		return this->reset(field.bpos, field.blength);
	}

	constexpr BitSet__& reset (const Enum auto pos, const Enum auto length) noexcept
	{
		return this->reset(std::to_underlying(pos), std::to_underlying(length));
	}

	constexpr BitSet__& reset (const Enum auto pos) noexcept
	{
		return this->reset(std::to_underlying(pos));
	}

	// --------------------------

	constexpr BitSet__& flip () noexcept
	{
		this->storage() = ~this->storage();
		this->ensure_safety_mask();
		return *this;
	}

	constexpr BitSet__& flip (const std::size_t pos, const std::size_t length) noexcept
	{
		const Type mask = ((static_cast<Type>(1) << length) - 1) << pos;
		this->storage() ^= mask;
		this->ensure_safety_mask();
		return *this;
	}

	constexpr BitSet__& flip (const std::size_t pos) noexcept
	{
		return this->flip(pos, 1);
	}

	constexpr BitSet__& flip (const BitField field) noexcept
	{
		return this->flip(field.bpos, field.blength);
	}

	constexpr BitSet__& flip (const Enum auto pos, const Enum auto length) noexcept
	{
		return this->flip(std::to_underlying(pos), std::to_underlying(length));
	}

	constexpr BitSet__& flip (const Enum auto pos) noexcept
	{
		return this->flip(std::to_underlying(pos));
	}
};

// ---------------------------------------------------

#define MYLIB_BUILD_OPERATION(OP) \
	template <typename ParentType, std::size_t nbits> \
	constexpr BitSet__<ParentType, nbits> operator OP (const BitSet__<ParentType, nbits> a, const BitSet__<ParentType, nbits> b) noexcept \
	{ \
		return BitSet__<ParentType, nbits>(a.to_underlying() OP b.to_underlying()); \
	} \
	template <typename ParentType, std::size_t nbits> \
	constexpr BitSet__<ParentType, nbits> operator OP (const BitSet__<ParentType, nbits> a, const std::integral auto v) noexcept \
	{ \
		return BitSet__<ParentType, nbits>(a.to_underlying() OP static_cast<typename ParentType::Type>(v)); \
	} \
	template <typename ParentType, std::size_t nbits> \
	constexpr BitSet__<ParentType, nbits> operator OP (const std::integral auto v, const BitSet__<ParentType, nbits> a) noexcept \
	{ \
		return BitSet__<ParentType, nbits>(static_cast<typename ParentType::Type>(v) OP a.to_underlying()); \
	}

MYLIB_BUILD_OPERATION( & )
MYLIB_BUILD_OPERATION( | )
MYLIB_BUILD_OPERATION( ^ )

#undef MYLIB_BUILD_OPERATION

// ---------------------------------------------------

#define MYLIB_BUILD_OPERATION(OP) \
	template <typename ParentType, std::size_t nbits> \
	constexpr BitSet__<ParentType, nbits> operator OP (const BitSet__<ParentType, nbits> a, const std::integral auto v) noexcept \
	{ \
		return BitSet__<ParentType, nbits>(a.to_underlying() OP v); \
	}

MYLIB_BUILD_OPERATION( << )
MYLIB_BUILD_OPERATION( >> )

#undef MYLIB_BUILD_OPERATION

// ---------------------------------------------------

template <std::size_t nbits>
using BitSet = BitSet__<BitSetStorage_< BitSetStorage__<calc_bit_set_storage_nbits__(nbits)> >, nbits>;

// ---------------------------------------------------

template <typename ParentType>
using BitSetT = BitSet__<ParentType, sizeof(ParentType) * 8>;

// ---------------------------------------------------

template <typename T>
class BitSetWrapper__ : public T
{
public:
	using Type = typename T::Type;

protected:
	Type& storage () noexcept
	{
		return *reinterpret_cast<Type*>(this);
	}

	const Type& storage () const noexcept
	{
		return *reinterpret_cast<const Type*>(this);
	}
};

// ---------------------------------------------------

template <typename T>
using BitSetWrapper = BitSetT< BitSetWrapper__<T> >;

// ---------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const BitField field)
{
	out << '{' << field.bpos << ',' << field.blength << '}';
	return out;
}

template <typename ParentType, std::size_t nbits>
std::ostream& operator << (std::ostream& out, const BitSet__<ParentType, nbits>& bitset)
{
	for (int32_t i = bitset.size() - 1; i >= 0; i--)
		out << static_cast<bool>(bitset[i]);
	return out;
}

// ---------------------------------------------------

#ifdef MYLIB_BUILD_OPERATION
	#undef MYLIB_BUILD_OPERATION
#endif

} // end namespace Mylib

#endif