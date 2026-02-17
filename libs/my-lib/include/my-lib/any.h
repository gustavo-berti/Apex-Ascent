#ifndef __MY_LIB_ANY_HEADER_H__
#define __MY_LIB_ANY_HEADER_H__

#include <type_traits>
#include <typeinfo>
#include <utility>
#include <any>

#include <cstdint>
#include <cstring>

#include <my-lib/macros.h>
#include <my-lib/std.h>

namespace Mylib
{

// ---------------------------------------------------

/*
	I don't like std::any because the C++ standard doesn't require
	it support static memory allocation.
	So I wrote this simpler version of it with static memory.
	The API is similar to std::any.
*/

template <std::size_t minimum_storage_size, std::size_t alignment=__STDCPP_DEFAULT_NEW_ALIGNMENT__>
class MYLIB_ALIGN_STRUCT(alignment) Any
{
public:
	consteval static std::size_t size () noexcept
	{
		return ((minimum_storage_size % alignment) == 0)
		? minimum_storage_size
		: static_cast<std::size_t>((minimum_storage_size + alignment) / alignment) * alignment;
	}

	// Friend declarations for any_cast functions
	template <typename T, std::size_t ms, std::size_t a>
	friend T any_cast(Any<ms, a>& any);
	
	template <typename T, std::size_t ms, std::size_t a>
	friend T any_cast(const Any<ms, a>& any);

private:
	static void foo_destroy (Any *self) { }

private:
	std::aligned_storage_t<size(), alignment> storage;
	const std::type_info *type = nullptr; // nullptr means empty Any
	void (*destroy)(Any*) = &Any::foo_destroy;
	void (*copy)(const Any*, Any*);
	void (*move)(Any*, Any*);

private:
	template <typename T>
	void copy_move_value (T&& value)
	{
		using Type = std::decay_t<T>;
		static_assert(sizeof(Type) <= size());

		new (&this->storage) Type(std::forward<T>(value));

		this->type = &typeid(Type);

		if constexpr (std::is_destructible_v<Type>) {
			this->destroy = [] (Any *self) {
				reinterpret_cast<Type*>(&self->storage)->~Type();
			};
		}
		else
			this->destroy = &Any::foo_destroy;

		if constexpr (std::is_copy_constructible_v<Type>) {
			this->copy = [] (const Any *source, Any *target) {
				new (&target->storage) Type(*(reinterpret_cast<const Type*>(&source->storage)));

				target->type = source->type;
				target->destroy = source->destroy;
				target->copy = source->copy;
				target->move = source->move;
			};
		}
		else
			this->copy = nullptr;

		if constexpr (std::is_move_constructible_v<Type>) {
			this->move = [] (Any *source, Any *target) {
				new (&target->storage) Type(std::move(*(reinterpret_cast<Type*>(&source->storage))));

				target->type = source->type;
				target->destroy = source->destroy;
				target->copy = source->copy;
				target->move = source->move;

				source->destroy(source);
				source->type = nullptr;
				source->destroy = &Any::foo_destroy;
				source->copy = nullptr;
				source->move = nullptr;
			};
		}
		else
			this->move = nullptr;
	}

public:
	Any () noexcept = default;

	~Any ()
	{
		this->reset();
	}

	void reset ()
	{
		this->destroy(this);
		this->type = nullptr;
		this->destroy = &Any::foo_destroy;
	}

	// -----------------------

	Any (const Any& other)
	{
		if (other.type == nullptr)
			return;
		
		other.copy(&other, this);
	}

	Any (Any&& other)
	{
		if (other.type == nullptr)
			return;
		
		other.move(&other, this);
	}

	template <typename T>
	requires (!std::same_as<std::decay_t<T>, Any>)
	Any (T&& value)
	{
		this->copy_move_value(std::forward<T>(value));
	}

	// -----------------------

	Any& operator= (const Any& other)
	{
		if (this == &other)
			return *this;
		
		this->destroy(this);

		if (other.type == nullptr) {
			this->type = nullptr;
			this->destroy = &Any::foo_destroy;
			this->copy = nullptr;
			this->move = nullptr;
			return *this;
		}
		
		other.copy(&other, this);
		return *this;
	}

	// -----------------------

	Any& operator= (Any&& other)
	{
		if (this == &other)
			return *this;
		
		this->destroy(this);

		if (other.type == nullptr) {
			this->type = nullptr;
			this->destroy = &Any::foo_destroy;
			this->copy = nullptr;
			this->move = nullptr;
			return *this;
		}
		
		other.move(&other, this);
		return *this;
	}

	// -----------------------

	template <typename T>
	requires (!std::same_as<std::decay_t<T>, Any>)
	Any& operator= (T&& value)
	{
		using Type = std::decay_t<T>;
		static_assert(sizeof(Type) <= size());

		if (this->type == &typeid(Type))
			*reinterpret_cast<std::decay_t<T>*>(&this->storage) = std::forward<T>(value);
		else {
			this->destroy(this);
			this->copy_move_value(std::forward<T>(value));
		}

		return *this;
	}

	// -----------------------

	void swap (Any& other) noexcept
	{
		if (this == &other)
			return;

		Any temp = std::move(*this);
		*this = std::move(other);
		other = std::move(temp);
	}

	// -----------------------

	template <typename ValueType, typename... Args>
	std::decay_t<ValueType>& emplace (Args&&... args)
	{
		this->destroy(this);
		this->copy_move_value(ValueType(std::forward<Args>(args)...));
		return *reinterpret_cast<std::decay_t<ValueType>*>(&this->storage);
	}

	// -----------------------

	bool has_value () const noexcept
	{
		return (this->type != nullptr);
	}

	// -----------------------

	const std::type_info& type_info () const noexcept
	{
		if (this->type == nullptr)
			return typeid(void);
		return *(this->type);
	}
};

// ---------------------------------------------------

template <typename T, std::size_t minimum_storage_size, std::size_t alignment>
T any_cast (Any<minimum_storage_size, alignment>& any)
{
	using Type = std::decay_t<T>;
	static_assert(sizeof(Type) <= minimum_storage_size);

	if (any.type == nullptr)
		throw std::bad_any_cast();

	if (*any.type != typeid(Type))
		throw std::bad_any_cast();

	Type *obj_ptr = reinterpret_cast<Type*>(&any.storage);

	if constexpr (std::is_rvalue_reference_v<T>)
		return std::move(*obj_ptr);
	else
		return *obj_ptr;
}

// ---------------------------------------------------

template <typename T, std::size_t minimum_storage_size, std::size_t alignment>
T any_cast (const Any<minimum_storage_size, alignment>& any)
{
	static_assert(!std::is_lvalue_reference_v<T> || (std::is_lvalue_reference_v<T> && std::is_const_v< std::remove_reference_t<T> >));
	static_assert(!std::is_rvalue_reference_v<T>);

	return any_cast<T, minimum_storage_size, alignment>
		           (const_cast<Any<minimum_storage_size, alignment>&>(any));
}

// ---------------------------------------------------

} // end namespace Mylib

#endif