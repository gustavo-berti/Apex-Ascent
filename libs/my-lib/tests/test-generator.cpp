#include <iostream>
#include <vector>
#include <string>

#include <cstdint>
#include <cassert>

#include <my-lib/generator.h>


using Mylib::StackGenerator;

int next_num (int current)
{
	return current + 1;
}


int main ()
{
	auto next = Mylib::make_stack_generator(0, 10, next_num);

	for (int val : next) {
		std::cout << val << " ";
	}

	std::cout << std::endl;

	for (const int val : next) {
		std::cout << val << " ";
	}

	std::cout << std::endl;

	auto fib = Mylib::make_stack_generator(1, 144, [prev = 0] (int current) mutable {
		int next = prev + current;
		prev = current;
		return next;
	});

	for (int val : fib) {
		std::cout << val << " ";
//		if (val > 100) break;  // Stop after exceeding 100
	}

	std::cout << std::endl;

	for (const int val : fib) {
		std::cout << val << " ";
//		if (val > 100) break;  // Stop after exceeding 100
	}

	std::cout << std::endl;

	return 0;
}