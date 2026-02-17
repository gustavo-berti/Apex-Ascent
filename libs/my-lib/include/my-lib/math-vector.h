#ifndef __MY_LIB_MATH_VECTOR_HEADER_H__
#define __MY_LIB_MATH_VECTOR_HEADER_H__

#include <iostream>
#include <concepts>
#include <type_traits>
#include <ostream>
#include <algorithm>
#include <numeric>
#include <numbers>

#include <cmath>

#include <my-lib/std.h>

namespace Mylib
{
namespace Math
{

#ifdef MYLIB_MATH_BUILD_OPERATION
#error nooooooooooo
#endif

// ---------------------------------------------------

template <typename T>
class Quaternion;

// ---------------------------------------------------

template <typename T, uint32_t dim_>
class VectorStorage__;

// ---------------------------------------------------

template <typename T>
class VectorStorage__<T, 2>
{
public:
	using Type = T;
	inline static constexpr uint32_t dim = 2;

	Type x;
	Type y;

	constexpr VectorStorage__ () noexcept = default;

	constexpr VectorStorage__ (const Type x_, const Type y_) noexcept
		: x(x_), y(y_)
	{
	}

	constexpr void set (const Type x, const Type y) noexcept
	{
		this->x = x;
		this->y = y;
	}

	constexpr Type* get_raw () noexcept { return &this->x; }
	constexpr const Type* get_raw () const noexcept { return &this->x; }

	constexpr Type& get (const uint32_t i) noexcept
	{
		if consteval {
			switch (i) {
				case 0: return this->x;
				case 1: return this->y;
			}
		}
		else {
			return this->get_raw()[i];
		}
	}

	constexpr Type get (const uint32_t i) const noexcept
	{
		if consteval {
			switch (i) {
				case 0: return this->x;
				case 1: return this->y;
			}
		}
		else {
			return this->get_raw()[i];
		}
	}
};

// ---------------------------------------------------

template <typename T>
class VectorStorage__<T, 3>
{
public:
	using Type = T;
	inline static constexpr uint32_t dim = 3;

	Type x;
	Type y;
	Type z;

	constexpr VectorStorage__ () noexcept = default;

	constexpr VectorStorage__ (const Type x_, const Type y_, const Type z_) noexcept
		: x(x_), y(y_), z(z_)
	{
	}

	constexpr void set (const Type x, const Type y, const Type z) noexcept
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	constexpr Type* get_raw () noexcept { return &this->x; }
	constexpr const Type* get_raw () const noexcept { return &this->x; }

	constexpr Type& get (const uint32_t i) noexcept
	{
		if consteval {
			switch (i) {
				case 0: return this->x;
				case 1: return this->y;
				case 2: return this->z;
			}
		}
		else {
			return this->get_raw()[i];
		}
	}

	constexpr Type get (const uint32_t i) const noexcept
	{
		if consteval {
			switch (i) {
				case 0: return this->x;
				case 1: return this->y;
				case 2: return this->z;
			}
		}
		else {
			return this->get_raw()[i];
		}
	}
};

// ---------------------------------------------------

template <typename T>
class VectorStorage__<T, 4>
{
public:
	using Type = T;
	inline static constexpr uint32_t dim = 4;

	Type x;
	Type y;
	Type z;
	Type w;

	constexpr VectorStorage__ () noexcept = default;

	constexpr VectorStorage__ (const Type x_, const Type y_, const Type z_, const Type w_) noexcept
		: x(x_), y(y_), z(z_), w(w_)
	{
	}

	constexpr void set (const Type x, const Type y, const Type z, const Type w) noexcept
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	constexpr Type* get_raw () noexcept { return &this->x; }
	constexpr const Type* get_raw () const noexcept { return &this->x; }

	constexpr Type& get (const uint32_t i) noexcept
	{
		if consteval {
			switch (i) {
				case 0: return this->x;
				case 1: return this->y;
				case 2: return this->z;
				case 3: return this->w;
			}
		}
		else {
			return this->get_raw()[i];
		}
	}

	constexpr Type get (const uint32_t i) const noexcept
	{
		if consteval {
			switch (i) {
				case 0: return this->x;
				case 1: return this->y;
				case 2: return this->z;
				case 3: return this->w;
			}
		}
		else {
			return this->get_raw()[i];
		}
	}
};

// ---------------------------------------------------

template <typename T>
class Vector : public T
{
public:
	using Type = typename T::Type;
	inline static constexpr uint32_t dim = T::dim;

	static_assert(sizeof(T) == (dim * sizeof(Type)));

	consteval static uint32_t get_dim () noexcept
	{
		return dim;
	}

	constexpr static Type fp (const auto v) noexcept
	{
		return static_cast<Type>(v);
	}

	// ------------------------ operator[]

	constexpr Type& operator[] (const uint32_t i) noexcept
	{
		static_assert(sizeof(Vector) == (dim * sizeof(Type)));
		return this->get(i);
	}

	constexpr Type operator[] (const uint32_t i) const noexcept
	{
		static_assert(sizeof(Vector) == (dim * sizeof(Type)));
		//return static_cast<const Type*>(this)[i];
		return this->get(i);
	}

	// ------------------------ Constructors

	using T::T;

	template <typename Tother>
	constexpr Vector (const Vector<Tother>& other, const Type other_values = 0) noexcept
	{
		static_assert(Tother::dim <= dim);
		auto& self = *this;
		for (uint32_t i = 0; i < Tother::dim; i++)
			self[i] = other[i];
		for (uint32_t i = Tother::dim; i < dim; i++)
			self[i] = other_values;
	}

	// ------------------------ operator=

	// use default ones, so no need to define them

	// ------------------------ Other stuff

	#undef MYLIB_MATH_BUILD_OPERATION
	#define MYLIB_MATH_BUILD_OPERATION(OP) \
		constexpr Vector& operator OP (this Vector& self, const Vector& other) noexcept \
		{ \
			for (uint32_t i = 0; i < dim; i++) \
				self[i] OP other[i]; \
			return self; \
		} \
		constexpr Vector& operator OP (this Vector& self, const Type s) noexcept \
		{ \
			for (uint32_t i = 0; i < dim; i++) \
				self[i] OP s; \
			return self; \
		}
	
	MYLIB_MATH_BUILD_OPERATION( += )
	MYLIB_MATH_BUILD_OPERATION( -= )
	MYLIB_MATH_BUILD_OPERATION( *= )
	MYLIB_MATH_BUILD_OPERATION( /= )


	constexpr Type length_squared (this const Vector& self) noexcept
	{
		Type value = 0;
		for (uint32_t i = 0; i < dim; i++)
			value += self[i] * self[i];
		return value;
	}

	constexpr Type length () const noexcept
	{
		return std::sqrt(this->length_squared());
	}

	constexpr void set_length (this Vector& self, const Type len) noexcept
	{
		const Type ratio = len / self.length();
		for (uint32_t i = 0; i < dim; i++)
			self[i] *= ratio;
	}

	// normalize returns the length of the vector before normalization.
	// Got the idea from the MathFu library.

	constexpr Type normalize (this Vector& self) noexcept
	{
		const Type len = self.length();
		for (uint32_t i = 0; i < dim; i++)
			self[i] /= len;
		return len;
	}

	constexpr void negate (this Vector& self) noexcept
	{
		for (uint32_t i = 0; i < dim; i++)
			self[i] = -self[i];
	}

	constexpr void invert () noexcept
	{
		this->negate();
	}

	constexpr void abs (this Vector& self) noexcept
	{
		for (uint32_t i = 0; i < dim; i++)
			self[i] = std::abs(self[i]);
	}

	constexpr void cross_product (this Vector& self, const Vector& a, const Vector& b) noexcept
		requires (dim == 3)
	{
		enum { x, y, z };
		self[x] = a[y] * b[z] - a[z] * b[y];
		self[y] = a[z] * b[x] - a[x] * b[z];
		self[z] = a[x] * b[y] - a[y] * b[x];
	}

	constexpr void rotate (const Quaternion<Type>& q) noexcept
	{
		const Quaternion<Type> v_(*this); // create a pure quaternion from the vector
		const Quaternion<Type> r = (q * v_) * conjugate(q);
		*this = r.v;
	}

	constexpr void project (const Vector& target) noexcept
	{
		*this = target * (dot_product(*this, target) / target.length_squared());
	}

	// Convertion functions that do not change the vector itself.

	constexpr Vector to_abs (this const Vector& self) noexcept
	{
		Vector v;
		for (uint32_t i = 0; i < dim; i++)
			v[i] = std::abs(self[i]);
		return v;
	}

	constexpr Vector to_normalized () const noexcept
	{
		return *this / this->length();
	}

	constexpr Vector to_length (const Type len) const noexcept
	{
		return *this * (len / this->length());
	}

	template <typename Tother>
	constexpr Tother to_reduced (this const Vector& self) noexcept
	{
		static_assert(Tother::dim < dim);
		Tother v;
		for (uint32_t i = 0; i < Tother::dim; i++)
			v[i] = self[i];
		return v;
	}

	// Functions to set the vector to pre-defined values.

	constexpr void set_zero (this Vector& self) noexcept
	{
		for (uint32_t i = 0; i < dim; i++)
			self[i] = 0;
	}

	static consteval Vector zero () noexcept
	{
		Vector v;
		v.set_zero();
		return v;
	}

	static constexpr Vector uniform (const Type value) noexcept
	{
		Vector v;
		
		for (uint32_t i = 0; i < dim; i++)
			v[i] = value;

		return v;
	}
};

// ---------------------------------------------------

template <typename T>
using Point = Vector<T>;

// ---------------------------------------------------

#undef MYLIB_MATH_BUILD_OPERATION
#define MYLIB_MATH_BUILD_OPERATION(OP) \
	template <typename T> \
	constexpr Vector<T> operator OP (const Vector<T>& a, const Vector<T>& b) noexcept \
	{ \
		Vector<T> r; \
		for (uint32_t i = 0; i < T::dim; i++) \
			r[i] = a[i] OP b[i]; \
		return r; \
	} \
	template <typename T> \
	constexpr Vector<T> operator OP (const Vector<T>& a, const typename T::Type s) noexcept \
	{ \
		Vector<T> r; \
		for (uint32_t i = 0; i < T::dim; i++) \
			r[i] = a[i] OP s; \
		return r; \
	} \
	template <typename T> \
	constexpr Vector<T> operator OP (const typename T::Type s, const Vector<T>& a) noexcept \
	{ \
		Vector<T> r; \
		for (uint32_t i = 0; i < T::dim; i++) \
			r[i] = s OP a[i]; \
		return r; \
	}

MYLIB_MATH_BUILD_OPERATION( + )
MYLIB_MATH_BUILD_OPERATION( - )
MYLIB_MATH_BUILD_OPERATION( * )
MYLIB_MATH_BUILD_OPERATION( / )


template <typename T> \
constexpr Vector<T> operator- (Vector<T> v) noexcept
{
	for (uint32_t i = 0; i < T::dim; i++) \
		v[i] = -v[i];
	return v;
}

// ---------------------------------------------------

template <typename T>
constexpr bool operator== (const Vector<T>& lhs, const Vector<T>& rhs) noexcept
{
	for (uint32_t i = 0; i < T::dim; i++)
		if (lhs[i] != rhs[i])
			return false;
	return true;
}

// ---------------------------------------------------

template <typename T>
constexpr bool operator!= (const Vector<T>& lhs, const Vector<T>& rhs) noexcept
{
	return !(lhs == rhs);
}

// ---------------------------------------------------

template <typename T>
constexpr auto dot_product (const Vector<T>& a, const Vector<T>& b) noexcept -> typename T::Type
{
	typename T::Type value = 0;
	for (uint32_t i = 0; i < T::dim; i++)
		value += a[i] * b[i];
	return value;
}

// ---------------------------------------------------

template <typename T>
constexpr auto cross_product (const Vector<T>& a, const Vector<T>& b) noexcept -> typename T::Type
	requires (T::dim == 2)
{
	enum { x, y };
	return a[x] * b[y] - a[y] * b[x];
}

// ---------------------------------------------------

template <typename T>
constexpr Vector<T> cross_product (const Vector<T>& a, const Vector<T>& b) noexcept
	requires (T::dim == 3)
{
	Vector<T> v;
	v.cross_product(a, b);
	return v;
}

// ---------------------------------------------------

template <typename T>
constexpr Vector<T> abs (Vector<T> v) noexcept
{
	return v.to_abs();
}

// ---------------------------------------------------

template <typename T>
constexpr Vector<T> max (const Vector<T>& a, const Vector<T>& b) noexcept
{
	Vector<T> r;
	for (uint32_t i = 0; i < T::dim; i++)
		r[i] = std::max(a[i], b[i]);
	return r;
}

// ---------------------------------------------------

template <typename T>
constexpr Vector<T> normalize (const Vector<T>& v) noexcept
{
	return v.to_normalized();
}

// ---------------------------------------------------

template <typename T>
constexpr Vector<T> with_length (const Vector<T>& v, const typename T::Type len) noexcept
{
	return v.to_length(len);
}

// ---------------------------------------------------

template <typename T>
constexpr Vector<T> projection (Vector<T> source, const Vector<T>& target) noexcept
{
	source.project(target);
	return source;
}

// ---------------------------------------------------

template <typename T>
constexpr auto distance (const Point<T>& a, const Point<T>& b) noexcept -> typename T::Type
{
	//static_assert(remove_type_qualifiers<Ta>::type::get_dim() == remove_type_qualifiers<Tb>::type::get_dim());
	return (a - b).length();
}

// ---------------------------------------------------

template <typename T>
constexpr auto distance_squared (const Point<T>& a, const Point<T>& b) noexcept -> typename T::Type
{
	return (a - b).length_squared();
}

// ---------------------------------------------------

template <typename T>
constexpr auto cos_angle_between (const Vector<T>& a, const Vector<T>& b) noexcept -> typename T::Type
{
	return dot_product(a, b) / (a.length() * b.length());
}

// ---------------------------------------------------

template <typename T>
constexpr auto angle_between (const Vector<T>& a, const Vector<T>& b) noexcept -> typename T::Type
{
	return std::acos(cos_angle_between(a, b));
}

// ---------------------------------------------------

template <typename T>
constexpr auto oriented_signed_angle_between (const Vector<T>& a, const Vector<T>& b) noexcept -> typename T::Type
	requires (T::dim == 2)
{
	return std::atan2(cross_product(a, b), dot_product(a, b));
}

// ---------------------------------------------------

template <typename T>
constexpr auto oriented_unsigned_angle_between (const Vector<T>& a, const Vector<T>& b) noexcept -> typename T::Type
	requires (T::dim == 2)
{
	using Type = typename T::Type;
	using Vector = Vector<T>;

	static constexpr Type value_to_add[2] = { Vector::fp(0), Vector::fp(2) * std::numbers::pi_v<Type> };

	const Type angle = oriented_signed_angle_between(a, b);
	const bool is_negative = (angle < Vector::fp(0));

	return angle + value_to_add[is_negative];
}

// ---------------------------------------------------

// Find an arbitrary vector that is orthogonal to the given vector.
// Copied from the MathFu library.

template <typename T>
constexpr Vector<T> orthogonal_vector (const Vector<T>& v) noexcept
	requires (T::dim == 3)
{
	// We start out by taking the cross product of the vector and the x-axis to
	// find something parallel to the input vectors.  If that cross product
	// turns out to be length 0 (i. e. the vectors already lie along the x axis)
	// then we use the y-axis instead.
	
	Vector<T> r = cross_product(Vector<T>(1, 0, 0), v);
	
	// We use a fairly high epsilon here because we know that if this number
	// is too small, the axis we'll get from a cross product with the y axis
	// will be much better and more numerically stable.
	
	if (r.length_squared() < static_cast<typename T::Type>(0.05))
		r = cross_product(Vector<T>(0, 1, 0), v);

	return r;
}

// ---------------------------------------------------

template <typename T>
struct VectorBasis3
{
	using VectorBasis = VectorBasis3;
	using Vector = Mylib::Math::Vector<VectorStorage__<T, 3>>;
	using Type = T;

	Vector vx;
	Vector vy;
	Vector vz;

	constexpr static uint32_t get_dim () noexcept
	{
		return 3;
	}

	// rh is right-handed

	constexpr void set_default_rh_orthonormal_basis () noexcept
	{
		this->vx = Vector(1, 0, 0);
		this->vy = Vector(0, 1, 0);
		this->vz = Vector(0, 0, 1);
	}

	constexpr void rotate (const Quaternion<Type>& q) noexcept
	{
		this->vx.rotate(q);
		this->vy.rotate(q);
		this->vz.rotate(q);
	}

	static constexpr VectorBasis default_rh_orthonormal_basis () noexcept
	{
		VectorBasis basis;
		basis.set_default_rh_orthonormal_basis();
		return basis;
	}
};

// ---------------------------------------------------

using Vector2f = Vector<VectorStorage__<float, 2>>;
using Point2f = Vector2f;

static_assert(sizeof(Vector2f) == (2 * sizeof(float)));

template <typename T>
concept is_Vector2f = std::same_as< typename remove_type_qualifiers<T>::type, Vector2f >;
//concept is_Vector2d = std::same_as<T, Vector2d> || std::same_as<T, Vector2d&> || std::same_as<T, Vector2d&&>;
//concept is_Vector2d = std::same_as< Vector2d, typename std::remove_cv< typename std::remove_reference<T>::type >::type >;
//concept is_Vector2d = std::same_as< typename std::remove_reference<T>::type, Vector2d >;

// ---------------------------------------------------

using Vector3f = Vector<VectorStorage__<float, 3>>;
using Point3f = Vector3f;

static_assert(sizeof(Vector3f) == (3 * sizeof(float)));

template <typename T>
concept is_Vector3f = std::same_as< typename remove_type_qualifiers<T>::type, Vector3f >;
//concept is_Vector2d = std::same_as<T, Vector2d> || std::same_as<T, Vector2d&> || std::same_as<T, Vector2d&&>;
//concept is_Vector2d = std::same_as< Vector2d, typename std::remove_cv< typename std::remove_reference<T>::type >::type >;
//concept is_Vector2d = std::same_as< typename std::remove_reference<T>::type, Vector2d >;

// ---------------------------------------------------

using Vector4f = Vector<VectorStorage__<float, 4>>;
using Point4f = Vector4f;

static_assert(sizeof(Vector4f) == (4 * sizeof(float)));

template <typename T>
concept is_Vector4f = std::same_as< typename remove_type_qualifiers<T>::type, Vector4f >;

// ---------------------------------------------------

template <typename T>
concept is_Vector = is_Vector2f<T> || is_Vector3f<T> || is_Vector4f<T>;

template <typename T>
concept is_Point = is_Vector<T>;

// ---------------------------------------------------

template <typename T>
std::ostream& operator << (std::ostream& out, const Vector<T>& v)
{
	out << "[";
	
	for (uint32_t i = 0; i < T::dim; i++) {
		out << v[i];
		
		if (i < (T::dim-1))
			out << ", ";
	}

	out << "]";

	return out;
}

// ---------------------------------------------------

template <typename T>
std::ostream& operator << (std::ostream& out, const VectorBasis3<T>& b)
{
	out << "[" << b.vx << ", " << b.vy << ", " << b.vz << "]";
	return out;
}

// ---------------------------------------------------

#undef MYLIB_MATH_BUILD_OPERATION

} // end namespace Math
} // end namespace Mylib

#endif