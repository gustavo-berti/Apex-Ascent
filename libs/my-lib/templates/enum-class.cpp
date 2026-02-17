#include <ostream>
#include <array>
#include <utility>

#include <my-lib/std.h>
#include <my-lib/exception.h>


#define _MYLIB_ENUM_CLASS_TYPE_VALUES_ \
	_MYLIB_ENUM_CLASS_TYPE_VALUE_(Value1) \
	_MYLIB_ENUM_CLASS_TYPE_VALUE_(Value2)

enum class Type : uint32_t {
	#define _MYLIB_ENUM_CLASS_TYPE_VALUE_(V) V,
	_MYLIB_ENUM_CLASS_TYPE_VALUES_
	#undef _MYLIB_ENUM_CLASS_TYPE_VALUE_
};

const char* enum_class_to_str (const Type value)
{
	static constexpr auto strs = std::to_array<const char*>({
		#define _MYLIB_ENUM_CLASS_TYPE_VALUE_(V) #V,
		_MYLIB_ENUM_CLASS_TYPE_VALUES_
		#undef _MYLIB_ENUM_CLASS_TYPE_VALUE_
	});

	using EnumType = typename Mylib::remove_type_qualifiers<decltype(value)>::type;
	using ExceptionType = typename Mylib::InvalidEnumClassValueException<EnumType>;

	mylib_assert_exception_args(std::to_underlying(value) < strs.size(), ExceptionType, value)

	return strs[ std::to_underlying(value) ];
}

inline std::ostream& operator << (std::ostream& out, const Type value)
{
	out << enum_class_to_str(value);
	return out;
}