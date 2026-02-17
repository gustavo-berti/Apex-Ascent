#include <iostream>
#include <vector>
#include <string>

#include <cstdint>
#include <cassert>

#include <my-lib/any.h>

using Any = Mylib::Any<801>;
using Mylib::any_cast;

int main ()
{
	Any data, data2;

	std::cout << "sizeof data is " << sizeof(data) << std::endl;

	data = 26;
	std::cout << "any has value " << any_cast<int32_t>(data) << std::endl;

	std::string name = "John";
	data = name;
	std::cout << "any has value " << any_cast<std::string>(data) << std::endl;

	data2 = data;

	std::cout << "any2 has value " << any_cast<std::string&>(data2) << std::endl;

	Any data3(std::move(data2));

	std::cout << "any3 has value " << any_cast<std::string>(data3) << std::endl;
	try {
		any_cast<std::string>(data2);
	}
	catch (const std::bad_any_cast& e) {
		std::cout << "\tany2 is empty after move: " << e.what() << std::endl;
	}

	const Any data4(data3);

	std::cout << "any4 has value " << any_cast<const std::string&>(data4) << std::endl;

	return 0;
}