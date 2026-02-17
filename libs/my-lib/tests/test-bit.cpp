#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include <cstdint>
#include <cassert>

#include <my-lib/bit.h>

struct BitField
{
	using Type = uint8_t;

	Type a : 2;
	Type b : 2;
	Type c : 4;
};

void test_bit_field ()
{
	using MyBitSet = Mylib::BitSetWrapper<BitField>;

	MyBitSet bitset;

	std::cout << "------------------------------------" << std::endl;
	std::cout << "Bit field test" << std::endl << std::endl;

	bitset = 0;
	std::cout << bitset << std::endl;

	bitset.b = 1;
	std::cout << bitset << std::endl;

	bitset.c = 1;
	std::cout << bitset << std::endl;
}

void test_enum ()
{
	using MyBitSet = Mylib::BitSet<16>;

	enum class menumc : uint8_t
	{
		a = 0,
		b = 1,
		c = 2,
		d = 3
	};

	enum menum
	{
		xa = 0,
		xb = 1,
		xc = 2,
		xd = 3
	};

	std::cout << "------------------------------------" << std::endl;
	std::cout << "Test ENUM" << std::endl << std::endl;

	MyBitSet bitset;

	std::cout << bitset << std::endl;

	bitset.set(xa, 1);
	std::cout << bitset << std::endl;

	bitset.set(menumc::b, 1);
	std::cout << bitset << std::endl;

	bitset.set({std::to_underlying(menumc::d), std::to_underlying(menumc::c)}, 3);
	std::cout << bitset << std::endl;

	const MyBitSet cset(bitset);
	std::cout << "bit xa: " << cset[xa] << std::endl;
	std::cout << "bit b: " << cset[menumc::b] << std::endl;
	std::cout << "bit [d, b]:" << cset[{std::to_underlying(menumc::d), std::to_underlying(menumc::c)}] << std::endl;
}

int main ()
{
	using MyBitSet = Mylib::BitSet<16>;

	MyBitSet bitset;
	bool bit;

	std::cout << "---------------------- a" << std::endl;
	bitset = 0x04;
	std::cout << bitset << std::endl;

	std::cout << "---------------------- b" << std::endl;
	bitset.set(0, 1);
	std::cout << bitset << std::endl;

	std::cout << "---------------------- c" << std::endl;
	const MyBitSet cset(4);
	bit = cset[0];
	std::cout << bit << std::endl;

	std::cout << "---------------------- d" << std::endl;
	bit = bitset[0];
	std::cout << bit << std::endl;

	std::cout << "---------------------- e" << std::endl;
	auto bbb = ~bitset(0, 2);
	std::cout << bbb << std::endl;

	std::cout << "---------------------- f" << std::endl;
	std::cout << bitset << std::endl;
	bitset.set({0, 2}, 0b0011);
	std::cout << bitset << std::endl;
	bitset.set(2, 0);
	std::cout << bitset << std::endl;

	std::cout << "---------------------- g" << std::endl;
	bitset.set({0, 2}, *bitset(2, 2));
	std::cout << bitset << std::endl;

	bitset = 0;

	std::cout << "---------------------- h" << std::endl;
	bitset.set({1, 4}, 0b1101);
	std::cout << bitset << std::endl;

#ifdef __cpp_multidimensional_subscript
	bitset = 0;

	std::cout << "---------------------- i" << std::endl;
	const Mylib::BitSet<3> bs (0b111);
	bitset.set({2, 3}, *bs);
	std::cout << bitset << std::endl;
#else
	#warning "Multidimensional subscripts are not supported"
#endif

	std::stringstream ss;
	std::cout << "---------------------- j" << std::endl;
	ss << bitset << std::endl;
	ss << ~bitset << std::endl;
	bitset = bitset[0];
	ss << bitset << std::endl;
	bitset.set(1, 1);
	bitset = bitset[1];
	ss << bitset << std::endl;
	std::cout << ss.str();

	std::cout << "---------------------- k" << std::endl;
	uint32_t v = 0xFFFFFFFF;
	std::cout << Mylib::BitSet<32>(Mylib::get_bits(v, 0, 31)) << std::endl;

	std::cout << "---------------------- l" << std::endl;
	bitset = MyBitSet(1) | MyBitSet(2);
	std::cout << bitset << std::endl;
	bitset = MyBitSet(0x00FF) & MyBitSet(0b00001100);
	std::cout << bitset << std::endl;
	bitset = MyBitSet(bitset) << 2;
	std::cout << bitset << std::endl;

	std::cout << "---------------------- m" << std::endl;
	bitset = 1;
	bitset |= 2;
	std::cout << bitset << std::endl;
	bitset = 0x00FF;
	bitset &= 0b00001100;
	std::cout << bitset << std::endl;
	bitset <<= 2;
	std::cout << bitset << std::endl;

	std::cout << "---------------------- n" << std::endl;
	bitset = 0;
	bitset.set(0, 16, 0xFFFF);
	std::cout << MyBitSet(bitset.get(0, 16)) << std::endl;

	test_bit_field();
	test_enum();

	return 0;
}