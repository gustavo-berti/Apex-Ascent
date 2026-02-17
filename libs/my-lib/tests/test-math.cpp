#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <array>
#include <iomanip>

#include <cstdint>
#include <cstdlib>
#include <cassert>

#include <my-lib/math.h>
#include <my-lib/math-vector.h>
#include <my-lib/math-matrix.h>
#include <my-lib/math-quaternion.h>

using namespace Mylib::Math;
using Matrix2f = Matrix<float, 2, 2>;
using Matrix3f = Matrix<float, 3, 3>;
using Matrix4f = Matrix<float, 4, 4>;

void test_vector_projection ()
{
	Vector2f source, target;

	std::cout << "---------------------- vector projection test" << std::endl << std::endl;

	source = Vector2f(4.0f, 2.0f);
	target = Vector2f(1, 0);
	std::cout << "source: " << source << " target: " << target << " projection: " << projection(source, target) << std::endl << std::endl;

	source = Vector2f(4.0f, 2.0f);
	target = Vector2f(-1, 0);
	std::cout << "source: " << source << " target: " << target << " projection: " << projection(source, target) << std::endl << std::endl;

	source = Vector2f(4.0f, 2.0f);
	target = Vector2f(1, 0.5);
	std::cout << "source: " << source << " target: " << target << " projection: " << projection(source, target) << std::endl << std::endl;

	source = Vector2f(4.0f, 2.0f);
	target = Vector2f(1, 1);
	std::cout << "source: " << source << " target: " << target << " projection: " << projection(source, target) << std::endl << std::endl;
}

void test_vector_angle ()
{
	std::cout << "---------------------- vector projection test" << std::endl << std::endl;

	constexpr auto a = Vector2f(1, 0);
	constexpr float angle_step = 30.0f;

	for (float angle = 0; angle <= 720; angle += angle_step) {
		const Matrix2f m = Matrix2f::rotation(degrees_to_radians(angle));
		const Vector2f b = m * a;
		std::cout << "angle: " << angle << " b: " << b << " angle_recalc " << radians_to_degrees(angle_between(a, b)) << " oriented_signed_angle " << radians_to_degrees(oriented_signed_angle_between(a, b)) << " oriented_unsigned_angle " << radians_to_degrees(oriented_unsigned_angle_between(a, b)) << std::endl;
	}
}

void test_matrix_lu_decomposition ()
{
	std::cout << std::setprecision(2);

	{
		auto m = Matrix2f(1.0f, 2.0f, 3.0f, 4.0f);
		std::cout << "Matrix:" << std::endl << m << std::endl;
		auto [pivot_indices, L, U, row_swaps] = m.to_LU_decomposition_pivoting();
		std::cout << "Pivot indices: ";
		for (const auto& index : pivot_indices)
			std::cout << index << ", ";
		std::cout << std::endl;
		std::cout << "L matrix:" << std::endl << L << std::endl;
		std::cout << "U matrix:" << std::endl << U << std::endl;
		std::cout << "P.A matrix:" << std::endl << (decltype(m)::pivot_matrix(pivot_indices) * m) << std::endl;
		std::cout << "L.U matrix:" << std::endl << (L * U) << std::endl;
		std::cout << std::endl;
	}

	{
		auto m = Matrix3f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
		std::cout << "Matrix:" << std::endl << m << std::endl;
		auto [pivot_indices, L, U, row_swaps] = m.to_LU_decomposition_pivoting();
		std::cout << "Pivot indices: ";
		for (const auto& index : pivot_indices)
			std::cout << index << ", ";
		std::cout << std::endl;
		std::cout << "L matrix:" << std::endl << L << std::endl;
		std::cout << "U matrix:" << std::endl << U << std::endl;
		std::cout << "P.A matrix:" << std::endl << (decltype(m)::pivot_matrix(pivot_indices) * m) << std::endl;
		std::cout << "L.U matrix:" << std::endl << (L * U) << std::endl;
		std::cout << std::endl;
	}

//	std::exit(0);
}

void test_matrix_determinant ()
{
	{
		auto m = Matrix2f(1.0f, 2.0f, 3.0f, 4.0f);
		std::cout << "Matrix:" << std::endl << m << std::endl;
		std::cout << "Determinant: " << m.determinant() << std::endl;
		std::cout << "Determinant laplace: " << m.determinant_laplace() << std::endl;
		std::cout << "Determinant gauss: " << m.determinant_gauss() << std::endl;
		auto [pivot_indices, L, U, row_swaps] = m.to_LU_decomposition_pivoting();
		std::cout << "Determinant LU: " << determinent_LU_pivoting(U, row_swaps) << std::endl;
		std::cout << std::endl;
	}
	{
		auto m = Matrix3f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
		std::cout << "Matrix:" << std::endl << m << std::endl;
		std::cout << "Determinant: " << m.determinant() << std::endl;
		std::cout << "Determinant laplace: " << m.determinant_laplace() << std::endl;
		std::cout << "Determinant gauss: " << m.determinant_gauss() << std::endl;
		auto [pivot_indices, L, U, row_swaps] = m.to_LU_decomposition_pivoting();
		std::cout << "Determinant LU: " << determinent_LU_pivoting(U, row_swaps) << std::endl;
		std::cout << std::endl;
	}
	{
		auto m = Matrix3f(11.0f, 21.0f, 61.0f, 41.0f, 51.0f, 91.0f, 71.0f, 81.0f, 91.0f);
		std::cout << "Matrix:" << std::endl << m << std::endl;
		std::cout << "Determinant: " << m.determinant() << std::endl;
		std::cout << "Determinant laplace: " << m.determinant_laplace() << std::endl;
		std::cout << "Determinant gauss: " << m.determinant_gauss() << std::endl;
		auto [pivot_indices, L, U, row_swaps] = m.to_LU_decomposition_pivoting();
		std::cout << "Determinant LU: " << determinent_LU_pivoting(U, row_swaps) << std::endl;
		std::cout << std::endl;
	}

//	std::exit(0);
}

void test_matrix_inverse ()
{
	{
		auto m = Matrix2f(1.0f, 2.0f, 3.0f, 4.0f);
		auto [inverse, success] = m.to_inverse();
		std::cout << "Matrix:" << std::endl << m << std::endl;
		std::cout << "Determinant: " << m.determinant() << std::endl;
		std::cout << "Inverse success: " << success << std::endl;
		if (success) {
			std::cout << "Inverse:" << std::endl << inverse << std::endl;
			std::cout << "Inverse * Matrix = Identity:" << std::endl;
			std::cout << (inverse * m) << std::endl;
		}
		std::cout << std::endl;
	}

	{
		auto m = Matrix3f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
		auto [inverse, success] = m.to_inverse();
		std::cout << "Matrix:" << std::endl << m << std::endl;
		std::cout << "Determinant: " << m.determinant() << std::endl;
		std::cout << "Inverse success: " << success << std::endl;
		if (success) {
			std::cout << "Inverse:" << std::endl << inverse << std::endl;
			std::cout << "Inverse * Matrix = Identity:" << std::endl;
			std::cout << (inverse * m) << std::endl;
		}
		std::cout << std::endl;
	}
}

int main ()
{
	Vector2f vzero = Vector2f::zero();
	Vector2f vv;

	float xxx = dot_product(vzero, vzero);
	std::cout << "xxx " << xxx << std::endl;

	std::array<Vector2f, 3> varray;

	union xx
	{
		Vector2f v1;
		Vector2f v2;

		xx () noexcept
		{
			v1.set(0, 0);
		}
	};

	xx uu;

	std::cout << "----------------------" << std::endl;
	Vector2f v1(2.0f, 3.0f);
	std::cout << v1 << std::endl;

	Matrix4f m;

	std::cout << "----------------------" << std::endl;
	m.set_identity();
	std::cout << m << std::endl;

	std::cout << "----------------------" << std::endl;
	m.set_scale(v1);
	std::cout << m << std::endl;

	std::cout << "----------------------" << std::endl;
	m.set_translate(v1);
	std::cout << m << std::endl;

	std::cout << "---------------------- m2" << std::endl;
	Matrix4f m2 = Matrix4f::identity();
	m2 *= Matrix4f::scale(v1);
	std::cout << m2 << std::endl;

//const float p[] = { 1.0f, 2.0f, 3.0f, 4.0f };
	std::cout << "---------------------- transpose" << std::endl;
	auto mt = Matrix2f({1.0f, 2.0f, 3.0f, 4.0f});
	std::cout << mt << std::endl;
	mt.transpose();
	std::cout << mt << std::endl;

	Vector3f axis = Vector3f(0.3f, 1.0f, 0.0f);
	float angle = degrees_to_radians(90.0f);
	Matrix3f m3 = Matrix3f::rotation(axis, angle);
	Quaternionf q1 = Quaternionf::rotation(axis, angle);
	Vector3f v2 = Vector3f(1.0f, 0.0f, 0.0f);
	std::cout << "Vector rotated with matrix: " << (m3 * v2) << " Vector rotated with quaternion: " << rotate(q1, v2) << std::endl;

	// test conversion

	{
		Quaternionf q2 = Quaternionf::rotation(Vector3f(1, 0, 0), Vector3f(0, 1, 0));
		auto [axis, angle] = q2.to_axis_angle();
		std::cout << "Quaternion to axis-angle: " << axis << " " << Mylib::Math::radians_to_degrees(angle) << std::endl;
	}

	test_vector_projection();
	test_vector_angle();

	std:: cout << "----------------------" << std::endl;
	std::cout << "Matrix LU decomposition:" << std::endl;
	test_matrix_lu_decomposition();

	std:: cout << "----------------------" << std::endl;
	std::cout << "Matrix determinent:" << std::endl;
	test_matrix_determinant();

	std:: cout << "----------------------" << std::endl;
	std::cout << "Matrix inverse:" << std::endl;
	test_matrix_inverse();

	std:: cout << "----------------------" << std::endl;

	for (int i = -2; i<= 8; i++) {
		std::cout << "log2_fast test " << i << " " << log2_fast(i) << std::endl;
		try {
			std::cout << "log2_safe test " << i << " " << log2_safe(i) << std::endl;
		}
		catch (const Mylib::InvalidNumberException& e) {
			std::cout << "log2_safe test " << i << " caught exception: " << std::endl << e.what() << std::endl;
		}
		std::cout << std::endl;
	}

	return 0;
}