#ifndef __MY_LIBS_MATRIX_HEADER_H__
#define __MY_LIBS_MATRIX_HEADER_H__

#include <cstdlib>
#include <cstring>

#include <span>
#include <vector>
#include <utility>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/exception.h>

namespace Mylib
{

// ---------------------------------------------------

template <typename T, uint32_t nrows, uint32_t ncols, bool bound_check = false>
class StaticMatrix
{
public:
	using Type = T;

private:
	T storage[nrows * ncols];

public:
	constexpr uint32_t get_nrows () noexcept
	{
		return nrows;
	}

	constexpr uint32_t get_ncols () noexcept
	{
		return ncols;
	}

	inline T* get_raw () noexcept
	{
		return this->storage;
	}

	inline const T* get_raw () const noexcept
	{
		return this->storage;
	}

	inline T& operator[] (const uint32_t row, const uint32_t col)
	{
		if constexpr (bound_check) {
			mylib_assert_exception_args(row < nrows, InvalidBoundaryException, row, nrows)
			mylib_assert_exception_args(col < ncols, InvalidBoundaryException, col, ncols)
		}
		return this->storage[row*ncols + col];
	}

	inline const T& operator[] (const uint32_t row, const uint32_t col) const
	{
		if constexpr (bound_check) {
			mylib_assert_exception_args(row < nrows, InvalidBoundaryException, row, nrows)
			mylib_assert_exception_args(col < ncols, InvalidBoundaryException, col, ncols)
		}
		return this->storage[row*ncols + col];
	}
};

// ---------------------------------------------------

template <typename T, bool bound_check = false>
class Matrix
{
public:
	using Type = T;

protected:
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, nrows, 0)
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, ncols, 0)
	std::vector<Type> storage;

public:
	Matrix () = default;

	Matrix (const uint32_t nrows_, const uint32_t ncols_)
		: nrows(nrows_), ncols(ncols_), storage(nrows_ * ncols_)
	{
	}

	Matrix (const uint32_t nrows_, const uint32_t ncols_, const Type& v)
		: nrows(nrows_), ncols(ncols_), storage(nrows_ * ncols_, v)
	{
	}

	~Matrix () = default;

	// copy-constructor
	Matrix (const Matrix& other)
		: nrows(other.nrows), ncols(other.ncols), storage(other.storage)
	{
	}

	// move constructor
	Matrix (Matrix&& other)
		: nrows(other.nrows), ncols(other.ncols), storage(std::move(other.storage))
	{
		other.nrows = 0;
		other.ncols = 0;
	}

	// copy-assign operator
	Matrix& operator= (const Matrix& other)
	{
		this->nrows = other.nrows;
		this->ncols = other.ncols;
		this->storage = other.storage;

		return *this;
	}

	// move-assign operator
	Matrix& operator= (Matrix&& other)
	{
		this->nrows = other.nrows;
		this->ncols = other.ncols;
		this->storage = std::move(other.storage);
		other.nrows = 0;
		other.ncols = 0;
		return *this;
	}

	void set_all (const Type& v)
	{
		const uint32_t size = this->nrows * this->ncols;

		for (uint32_t i = 0; i < size; i++)
			this->storage[i] = v;
	}

	Type* get_raw () noexcept
	{
		return this->storage.data();
	}

	const Type* get_raw () const noexcept
	{
		return this->storage.data();
	}

	Type& operator[] (const uint32_t row, const uint32_t col)
	{
		if constexpr (bound_check) {
			mylib_assert_exception_args(row < this->nrows, InvalidBoundaryException, row, this->nrows)
			mylib_assert_exception_args(col < this->ncols, InvalidBoundaryException, col, this->ncols)
		}
		return this->storage[row*this->ncols + col];
	}

	const Type& operator[] (const uint32_t row, const uint32_t col) const
	{
		if constexpr (bound_check) {
			mylib_assert_exception_args(row < this->nrows, InvalidBoundaryException, row, this->nrows)
			mylib_assert_exception_args(col < this->ncols, InvalidBoundaryException, col, this->ncols)
		}
		return this->storage[row*this->ncols + col];
	}

	std::span<Type> to_span () noexcept
	{
		return std::span<Type>(this->storage.data(), this->nrows * this->ncols);
	}
};

// ---------------------------------------------------

} // end namespace Mylib

#endif