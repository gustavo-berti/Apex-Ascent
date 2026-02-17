#ifndef __MY_LIB_EXCEPTION_HEADER_H__
#define __MY_LIB_EXCEPTION_HEADER_H__

#include <sstream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <source_location>
#include <tuple>

#include <my-lib/std.h>


namespace Mylib
{

// ---------------------------------------------------

class Exception : public std::exception
{
private:
	mutable std::string msg;
	std::source_location location;
	const char *assert_str;
	const char *extra_msg;

public:
	Exception (const std::source_location& location_, const char *assert_str_, const char *extra_msg_)
		: location(location_), assert_str(assert_str_), extra_msg(extra_msg_)
	{
	}

	const char* what () const noexcept override final
	{
		try {
			std::ostringstream str_stream;

			str_stream << "My-lib Exception: "
				<< "File: " << this->location.file_name() << std::endl
				<< "Line: " << this->location.line() << std::endl
				<< "Function: " << this->location.function_name() << std::endl;

			if (this->assert_str != nullptr)
				str_stream << "Assertion that failed: " << this->assert_str << std::endl;
	
			this->build_exception_msg(str_stream);
			str_stream << std::endl;

			if (this->extra_msg != nullptr)
				str_stream << "Extra message: " << this->extra_msg << std::endl;

			this->msg = str_stream.str();
	
			return this->msg.c_str();
		} catch (...) {
			return "My-lib Exception: error while building exception message";
		}
	}

protected:
	virtual void build_exception_msg (std::ostringstream& str_stream) const = 0;
};

// ---------------------------------------------------

class AssertException : public Exception
{
public:
	AssertException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_)
		: Exception(location_, assert_str_, extra_msg_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Assert exception.";
	}
};

// ---------------------------------------------------

template <typename... Args>
auto make_assert_exception_msg__ (const std::source_location& location, const char *assert_str, Args&&... args)
{
	auto packed_args = std::make_tuple(args...);
	using Tpacked_args = decltype(packed_args);

	class AssertExceptionMsg : public Exception
	{
	private:
		Tpacked_args args;

	public:
		AssertExceptionMsg (const std::source_location& location_, const char *assert_str_, const Tpacked_args& args_)
			: Exception(location_, assert_str_, nullptr), args(args_)
		{
		}

	protected:
		void build_exception_msg (std::ostringstream& str_stream) const override final
		{
			str_stream << "Assert exception with message:" << std::endl;
			std::apply([&str_stream] (auto&&... args) -> void {
				((str_stream << args), ...);
			}, this->args);
		}
	};

	return AssertExceptionMsg(location, assert_str, packed_args);
}

// ---------------------------------------------------

class InvalidNumberException : public Exception
{
public:
	InvalidNumberException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_)
		: Exception(location_, assert_str_, extra_msg_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Invalid number exception.";
	}
};

// ---------------------------------------------------

class ZeroNumberException : public Exception
{
public:
	ZeroNumberException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_)
		: Exception(location_, assert_str_, extra_msg_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Zero number exception.";
	}
};

// ---------------------------------------------------

class SingularMatrixException : public Exception
{
public:
	SingularMatrixException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_)
		: Exception(location_, assert_str_, extra_msg_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Singular matrix exception.";
	}
};

// ---------------------------------------------------

class EventSubscriberNotFoundException : public Exception
{
public:
	EventSubscriberNotFoundException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_)
		: Exception(location_, assert_str_, extra_msg_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Event subscriber not found exception.";
	}
};

// ---------------------------------------------------

class InvalidBoundaryException : public Exception
{
private:
	size_t pos;
	size_t max;

	public:
	InvalidBoundaryException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_, const size_t pos_, const size_t max_)
		: Exception(location_, assert_str_, extra_msg_), pos(pos_), max(max_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Invalid boundary exception: "
			<< " Pos: " << this->pos
			<< " Max: " << this->max;
	}
};

// ---------------------------------------------------

class PoolAllocatorMultiException : public Exception
{
private:
	size_t type_size;
	size_t count;
	size_t alignment;

	public:
	PoolAllocatorMultiException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_, const size_t type_size_, const size_t count_, const size_t alignment_)
		: Exception(location_, assert_str_, extra_msg_), type_size(type_size_), count(count_), alignment(alignment_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Pool Allocator multi-element allocation exception: "
			<< " Type size: " << this->type_size
			<< " Count: " << this->count
			<< " Alignment: " << this->alignment << std::endl
			<< " This allocator only supports allocation/de-allocation of one element at a time.";
	}
};

// ---------------------------------------------------

template <Mylib::Enum T>
class InvalidEnumClassValueException : public Exception
{
private:
	T value;

public:
	InvalidEnumClassValueException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_, const T value_)
		: Exception(location_, assert_str_, extra_msg_), value(value_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Invalid enum class value exception: "
			<< " Value: " << std::to_underlying(this->value);
	}
};

// ---------------------------------------------------

/*
// This is a helper function to throw an exception with no arguments.

template <typename Texception>
void throw_exception (const std::source_location& location = std::source_location::current())
{
	throw Texception(location);
}

template <typename Texception>
void assert_exception (const bool bool_expr, const std::source_location& location = std::source_location::current())
{
	if (!bool_expr) [[unlikely]]
		throw_exception<Texception>(location);
}

// ---------------------------------------------------

// This is a helper function to throw an exception with 1 argument.

template <typename Texception, typename Ta>
void throw_exception (Ta&& arg_a, const std::source_location& location = std::source_location::current())
{
	throw Texception(location, std::forward<Ta>(arg_a));
}

template <typename Texception, typename Ta>
void assert_exception (const bool bool_expr, Ta&& arg_a, const std::source_location& location = std::source_location::current())
{
	if (!bool_expr) [[unlikely]]
		throw_exception<Texception>(location, std::forward<Ta>(arg_a));
}

// ---------------------------------------------------

// This is a helper function to throw an exception with 2 arguments.

template <typename Texception, typename Ta, typename Tb>
void throw_exception (Ta&& arg_a, Tb&& arg_b, const std::source_location& location = std::source_location::current())
{
	throw Texception(location, std::forward<Ta>(arg_a), std::forward<Tb>(arg_b));
}

template <typename Texception, typename Ta, typename Tb>
void assert_exception (const bool bool_expr, Ta&& arg_a, Tb&& arg_b, const std::source_location& location = std::source_location::current())
{
	if (!bool_expr) [[unlikely]]
		throw_exception<Texception>(location, std::forward<Ta>(arg_a), std::forward<Tb>(arg_b));
}

// This is a helper function to throw an exception with 3 arguments.

template <typename Texception, typename Ta, typename Tb, typename Tc>
void throw_exception (Ta&& arg_a, Tb&& arg_b, Tc&& arg_c, const std::source_location& location = std::source_location::current())
{
	throw Texception(location, std::forward<Ta>(arg_a), std::forward<Tb>(arg_b), std::forward<Tc>(arg_c));
}

template <typename Texception, typename Ta, typename Tb, typename Tc>
void assert_exception (const bool bool_expr, Ta&& arg_a, Tb&& arg_b, Tc&& arg_c, const std::source_location& location = std::source_location::current())
{
	if (!bool_expr) [[unlikely]]
		throw_exception<Texception>(location, std::forward<Ta>(arg_a), std::forward<Tb>(arg_b), std::forward<Tc>(arg_c));
}
*/
// ---------------------------------------------------

/*template <typename Texception, typename... Targs>
void throw_exception__ (const std::source_location& location, const char *assert_str, Targs&&... args)
{
	throw Texception(location, assert_str, std::forward<Targs>(args)...);
}*/

/*template <typename Texception, typename... Targs>
void assert_exception (const bool bool_expr, const std::source_location& location = std::source_location::current(), Targs&&... args)
{
	if (!bool_expr) [[unlikely]]
		throw_exception<Texception>(location, std::forward<Targs>(args)...);
}*/

// ---------------------------------------------------

#define mylib_throw(EXCEPTION_TYPE) \
	throw EXCEPTION_TYPE(std::source_location::current(), nullptr, nullptr)

#define mylib_throw_msg(EXCEPTION_TYPE, msg) \
	throw EXCEPTION_TYPE(std::source_location::current(), nullptr, msg)

#define mylib_throw_args(EXCEPTION_TYPE, ...) \
	throw EXCEPTION_TYPE(std::source_location::current(), nullptr, nullptr, __VA_ARGS__)

#define mylib_throw_msg_args(EXCEPTION_TYPE, msg, ...) \
	throw EXCEPTION_TYPE(std::source_location::current(), nullptr, msg, __VA_ARGS__)

#define mylib_assert_exception(bool_expr, EXCEPTION_TYPE) { \
		if (!(bool_expr)) [[unlikely]] \
			throw EXCEPTION_TYPE(std::source_location::current(), #bool_expr, nullptr); \
	}

#define mylib_assert_exception_msg(bool_expr, EXCEPTION_TYPE, msg) { \
		if (!(bool_expr)) [[unlikely]] \
			throw EXCEPTION_TYPE(std::source_location::current(), #bool_expr, msg); \
	}

#define mylib_assert_exception_args(bool_expr, EXCEPTION_TYPE, ...) { \
		if (!(bool_expr)) [[unlikely]] \
			throw EXCEPTION_TYPE(std::source_location::current(), #bool_expr, nullptr, __VA_ARGS__); \
	}
	
#define mylib_assert_exception_msg_args(bool_expr, EXCEPTION_TYPE, msg, ...) { \
		if (!(bool_expr)) [[unlikely]] \
			throw EXCEPTION_TYPE(std::source_location::current(), #bool_expr, msg, __VA_ARGS__); \
	}

#define mylib_assert(bool_expr) \
	mylib_assert_exception((bool_expr), Mylib::AssertException)

#define mylib_throw_assert_msg(...) \
	throw Mylib::make_assert_exception_msg__(std::source_location::current(), nullptr, __VA_ARGS__)

#define mylib_assert_msg(bool_expr, ...) { \
		if (!(bool_expr)) [[unlikely]]\
			throw Mylib::make_assert_exception_msg__(std::source_location::current(), #bool_expr, __VA_ARGS__); \
	}

// ---------------------------------------------------

} // end namespace Mylib

#endif