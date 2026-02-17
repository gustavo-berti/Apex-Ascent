#ifndef __MY_LIB_GENERATOR_HEADER_H__
#define __MY_LIB_GENERATOR_HEADER_H__

#include <my-lib/macros.h>
#include <my-lib/std.h>

namespace Mylib
{

// ---------------------------------------------------

// still have to work on this
// for now it is only a draft I designed using Deepseek

#include <iostream>

template <typename T, typename NextFn>
class StackGenerator
{
public:
	using Type = T;

private:
	const Type init_value; // initial value of the generator
	const Type end_value;  // end value of the generator
	const NextFn next_fn;  // State transition logic

public:
	StackGenerator (const Type& init_value_, const Type& end_value_, const NextFn& next_fn_)
		: init_value(init_value_), end_value(end_value_), next_fn(next_fn_)
	{
	}

	class Iterator
	{
	private:
		const StackGenerator *parent;
		Type current;
		NextFn next_fn;  // State transition logic
	
	public:
		Iterator (const StackGenerator *parent_, const Type& value_, const NextFn& next_fn_)
			: parent(parent_), current(value_), next_fn(next_fn_)
		{
		}

		Iterator& operator++ ()
		{
			this->current = this->next_fn(this->current);
			return *this;
		}

		const Type& operator* () const { return this->current; }
		
		bool operator!= (const Iterator& other) const
		{
			return (this->current != other.current);
		}
	};

	Iterator begin () const { return Iterator(this, this->init_value, this->next_fn); }
	Iterator end () const { return Iterator(this, this->end_value, this->next_fn); }
};

// ---------------------------------------------------

template <typename T, typename NextFn__>
auto make_stack_generator (const T& init_value, const T& end_value, const NextFn__& next_fn)
{
	using NextFn = std::conditional_t<std::is_function_v<NextFn__>,
	                                  NextFn__&,
	                                  NextFn__>;

	return StackGenerator<T, NextFn>(init_value, end_value, next_fn);
}

// ---------------------------------------------------

} // end namespace Mylib

#endif