#ifndef __MY_LIB_MATH_QUATERNION_HEADER_H__
#define __MY_LIB_MATH_QUATERNION_HEADER_H__

#include <concepts>
#include <type_traits>
#include <ostream>
#include <utility>

#include <cmath>

#include <my-lib/std.h>
#include <my-lib/math-vector.h>
#include <my-lib/math-matrix.h>

namespace Mylib
{
namespace Math
{

#ifdef MYLIB_MATH_BUILD_OPERATION
#error nooooooooooo
#endif

// ---------------------------------------------------

template <typename T>
class Quaternion
{
public:
	using Type = T;
	using Vector = Mylib::Math::Vector<VectorStorage__<Type, 3>>;

	constexpr static Type fp (const auto v) noexcept
	{
		return static_cast<Type>(v);
	}

	// We store in this format (x, y, z, w), where x,y,z are the vector part,
	// and w is the scalar part, so that we can use the same memory layout
	// as in GLSL.

	Vector v;
	Type w;

	// ------------------------ Constructors

	constexpr Quaternion () noexcept = default;

	constexpr Quaternion (const Vector& v_, const T w_) noexcept
		: v(v_), w(w_)
	{
	}

	constexpr Quaternion (const Type x_, const Type y_, const Type z_, const Type w_) noexcept
		: v(x_, y_, z_), w(w_)
	{
	}

	// Create a scalar (or real) quaternion.
	// The vector part is set to zero.

	constexpr Quaternion (const Type w_) noexcept
		: v(Vector::zero()), w(w_)
	{
	}

	// Create a vector (or imaginary, or pure) quaternion.
	// The scalar part is set to zero.

	constexpr Quaternion (const Vector& v_) noexcept
		: v(v_), w(0)
	{
	}

	// ------------------------ operator=

	// use default ones, so no need to define them

	// ------------------------ Other stuff

	#undef MYLIB_MATH_BUILD_OPERATION
	#define MYLIB_MATH_BUILD_OPERATION(OP) \
		constexpr Quaternion& operator OP (this Quaternion& self, const Quaternion& other) noexcept \
		{ \
			self.v OP other.v; \
			self.w OP other.w; \
			return self; \
		}
	
	MYLIB_MATH_BUILD_OPERATION( += )
	MYLIB_MATH_BUILD_OPERATION( -= )

	#undef MYLIB_MATH_BUILD_OPERATION
	#define MYLIB_MATH_BUILD_OPERATION(OP) \
		constexpr Quaternion& operator OP (this Quaternion& self, const Type s) noexcept \
		{ \
			self.v OP s; \
			self.w OP s; \
			return self; \
		}
	
	MYLIB_MATH_BUILD_OPERATION( *= )
	MYLIB_MATH_BUILD_OPERATION( /= )

	// ---------------------------------------------------

	constexpr Quaternion& operator *= (const Quaternion& other) noexcept
	{
		*this = *this * other;
		return *this;
	}

	// ---------------------------------------------------

	constexpr Type length_squared (this const Quaternion& self) noexcept
	{
		Type value = dot_product(self.v, self.v);
		value += self.w * self.w;
		return value;
	}

	constexpr Type length () const noexcept
	{
		return std::sqrt(this->length_squared());
	}

	// ---------------------------------------------------

	/*
		Converts the quaternion to a pair of a normalized axis-vector and an angle.
		It considers the quaternion as a rotation quaternion (unit quaternion length=1).
	*/

	constexpr std::pair<Vector, Type> to_axis_angle () const noexcept
	{
		Quaternion<Type> q = (this->w > 0) ? *this : Quaternion<Type>(-(*this));
		Vector axis;
		Type angle;
		
		const Type length = q.v.normalize();

		// In case length is zero, normalize leaves NaNs in axis.
		// This happens at angle = 0 and 360.
		// All axes are correct, so any will do.

		if (length == fp(0)) [[unlikely]]
			axis.set(1, 0, 0);
		else
			axis = q.v;

		angle = fp(2) * std::atan2(length, this->w);
		// angle = std::acos(this->w) * fp(2);

		return std::make_pair(axis, angle);

/*		const T angle = std::acos(this->w) * 2;
		const T s = std::sqrt(1 - this->w * this->w);

		if (s < 0.001f)
			return std::make_pair(Vector(1, 0, 0), 0);

		return std::make_pair(this->v / s, angle);*/
	}

	// ---------------------------------------------------

	// Returns the rotation matrix corresponding to this quaternion.

	template <uint32_t dim>
	constexpr Matrix<Type, dim, dim> to_rotation_matrix () const noexcept
		requires (dim == 3 || dim == 4)
	{
		Matrix<Type, dim, dim> m;

		const Type x2 = this->v.x * this->v.x;
		const Type y2 = this->v.y * this->v.y;
		const Type z2 = this->v.z * this->v.z;
		const Type xy = this->v.x * this->v.y;
		const Type xz = this->v.x * this->v.z;
		const Type yz = this->v.y * this->v.z;
		const Type wx = this->w * this->v.x;
		const Type wy = this->w * this->v.y;
		const Type wz = this->w * this->v.z;
		
		m[0, 0] = fp(1) - fp(2) * (y2 + z2);
		m[0, 1] = fp(2) * (xy - wz);
		m[0, 2] = fp(2) * (xz + wy);

		m[1, 0] = fp(2) * (xy + wz);
		m[1, 1] = fp(1) - fp(2) * (x2 + z2);
		m[1, 2] = fp(2) * (yz - wx);

		m[2, 0] = fp(2) * (xz - wy);
		m[2, 1] = fp(2) * (yz + wx);
		m[2, 2] = fp(1) - fp(2) * (x2 + y2);

		if constexpr (dim == 4) {
			m[0, 3] = 0;
			m[1, 3] = 0;
			m[2, 3] = 0;

			m[3, 0] = 0;
			m[3, 1] = 0;
			m[3, 2] = 0;
			m[3, 3] = 1;
		}

		return m;
	}

	// ---------------------------------------------------

	constexpr void set_rotation (const Vector& axis, const Type angle) noexcept
	{
		const Type half_angle = angle / fp(2);
		this->v = Mylib::Math::normalize(axis) * std::sin(half_angle);
		this->w = std::cos(half_angle);
	}

	// ---------------------------------------------------

	constexpr void set_rotation (Vector start, Vector end) noexcept
	{
		start.normalize();
		end.normalize();

		const Type dot_product = Mylib::Math::dot_product(start, end);

		// From MathFu library:
		// Any rotation < 0.1 degrees is treated as no rotation
		// in order to avoid division by zero errors.
		// So we early-out in cases where it's less than 0.1 degrees.
		// cos( 0.1 degrees) = 0.99999847691

		if (dot_product >= fp(0.99999847691)) {
			this->set_identity();
			return;
		}

		// From MathFu library:
		// If the vectors point in opposite directions, return a 180 degree
		// rotation, on an arbitrary axis.

		if (dot_product <= fp(-0.99999847691)) {
			this->v = orthogonal_vector(start);
			this->w = 0;
			return;
		}
		
		// Degenerate cases have been handled, so if we're here, we have to
		// actually compute the angle we want.

		this->v = cross_product(start, end);
		this->w = fp(1.0) + dot_product;

		this->normalize();
	}

	// ---------------------------------------------------

	// normalize returns the length before normalization.
	// Got the idea from the MathFu library.

	constexpr Type normalize (this Quaternion& self) noexcept
	{
		const Type len = self.length();
		self.v /= len;
		self.w /= len;
		return len;
	}

	constexpr void conjugate () noexcept
	{
		this->v.negate();
	}

	/*
		If the quaternion is normalized, then its inverse is equal to its conjugate.
	*/

	constexpr void invert_normalized () noexcept
	{
		this->conjugate();
	}

	constexpr void invert (this Quaternion& self) noexcept
	{
		const Type len = self.length_squared();
		self.conjugate();
		self.v /= len;
		self.w /= len;
	}

	// ---------------------------------------------------

	constexpr void set_zero () noexcept
	{
		this->v.set_zero();
		this->w = 0;
	}

	constexpr void set_identity () noexcept
	{
		this->v.set_zero();
		this->w = 1;
	}

	// ---------------------------------------------------

	static consteval Quaternion zero () noexcept
	{
		Quaternion q;
		q.set_zero();
		return q;
	}

	static consteval Quaternion identity () noexcept
	{
		Quaternion q;
		q.set_identity();
		return q;
	}

	static constexpr Quaternion rotation (const Vector& axis, const Type angle) noexcept
	{
		Quaternion q;
		q.set_rotation(axis, angle);
		return q;
	}

	static constexpr Quaternion rotation (const Vector& start, const Vector& end) noexcept
	{
		Quaternion q;
		q.set_rotation(start, end);
		return q;
	}
};

// ---------------------------------------------------

using Quaternionf = Quaternion<float>;
using Quaterniond = Quaternion<double>;

static_assert(sizeof(Quaternionf) == (4 * sizeof(float)));
static_assert(sizeof(Quaterniond) == (4 * sizeof(double)));

// ---------------------------------------------------

#undef MYLIB_MATH_BUILD_OPERATION
#define MYLIB_MATH_BUILD_OPERATION(OP) \
	template <typename T> \
	constexpr Quaternion<T> operator OP (const Quaternion<T>& a, const Quaternion<T>& b) noexcept \
	{ \
		Quaternion<T> r; \
		r.v = a.v OP b.v; \
		r.w = a.w OP b.w; \
		return r; \
	}

MYLIB_MATH_BUILD_OPERATION( + )
MYLIB_MATH_BUILD_OPERATION( - )

#undef MYLIB_MATH_BUILD_OPERATION
#define MYLIB_MATH_BUILD_OPERATION(OP) \
	template <typename T> \
	constexpr Quaternion<T> operator OP (const Quaternion<T>& a, const T s) noexcept \
	{ \
		Quaternion<T> r; \
		r.v = a.v OP s; \
		r.w = a.w OP s; \
		return r; \
	}

MYLIB_MATH_BUILD_OPERATION( * )
MYLIB_MATH_BUILD_OPERATION( / )

// ---------------------------------------------------

template <typename T>
constexpr Quaternion<T> operator- (const Quaternion<T>& q) noexcept
{
	Quaternion<T> r;
	r.v = -q.v;
	r.w = -q.w;
	return r;
}

// ---------------------------------------------------

template <typename T>
constexpr Quaternion<T> normalize (const Quaternion<T>& q) noexcept
{
	return q / q.length();
}

// ---------------------------------------------------

template <typename T>
constexpr Quaternion<T> conjugate (const Quaternion<T>& q) noexcept
{
	return Quaternion<T>(-q.v, q.w);
}

// ---------------------------------------------------

/*
	If the quaternion is normalized, then its inverse is equal to its conjugate.
*/

template <typename T>
constexpr Quaternion<T> invert_normalized (const Quaternion<T>& q) noexcept
{
	return conjugate(q);
}

template <typename T>
constexpr Quaternion<T> invert (const Quaternion<T>& q) noexcept
{
	return conjugate(q) / q.length_squared();
}

// ---------------------------------------------------

template <typename T>
constexpr Quaternion<T> operator* (const Quaternion<T>& q1, const Quaternion<T>& q2) noexcept
{
	Quaternion<T> r;

	// Hamilton product

	enum { x, y, z };

	r.v[x] = (q1.w * q2.v[x]) + (q1.v[x] * q2.w) + (q1.v[y] * q2.v[z]) - (q1.v[z] * q2.v[y]);
	r.v[y] = (q1.w * q2.v[y]) - (q1.v[x] * q2.v[z]) + (q1.v[y] * q2.w) + (q1.v[z] * q2.v[x]);
	r.v[z] = (q1.w * q2.v[z]) + (q1.v[x] * q2.v[y]) - (q1.v[y] * q2.v[x]) + (q1.v[z] * q2.w);
	r.w = (q1.w * q2.w) - (q1.v[x] * q2.v[x]) - (q1.v[y] * q2.v[y]) - (q1.v[z] * q2.v[z]);
	//r.v = (b.v * a.w) + (a.v * b.w) + cross_product(a.v, b.v);
	//r.w = a.w * b.w - dot_product(a.v, b.v);

	return r;
}

// ---------------------------------------------------

template <typename Tvector>
constexpr Vector<Tvector> rotate (const Quaternion<typename Tvector::Type>& q, Vector<Tvector> v) noexcept
	requires (Tvector::dim == 3)
{
	v.rotate(q);
	return v;
}

// ---------------------------------------------------

template <typename T>
std::ostream& operator << (std::ostream& out, const Quaternion<T>& q)
{
	enum { x, y, z };
	out << "[" << q.v[x] << ", " << q.v[y] << ", " << q.v[z] << ", " << q.w << "]";
	return out;
}

// ---------------------------------------------------

#undef MYLIB_MATH_BUILD_OPERATION

} // end namespace Math
} // end namespace Mylib

#endif