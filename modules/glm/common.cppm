module;

#include <glm/common.hpp>
#include <glm/integer.hpp>
#include <glm/trigonometric.hpp>
#include <glm/exponential.hpp>
#include <glm/gtc/constants.hpp>

export module glm.common;

export namespace glm
{
	// Common

	/// Returns x if x >= 0; otherwise, it returns -x.
	using glm::abs;

	/// Returns 1.0 if x > 0, 0.0 if x == 0, or -1.0 if x < 0.
	using glm::sign;

	/// Returns a value equal to the nearest integer that is less then or equal to x.
	using glm::floor;

	/// Returns a value equal to the nearest integer to x
	/// whose absolute value is not larger than the absolute value of x.
	using glm::trunc;

	/// Returns a value equal to the nearest integer to x.
	/// The fraction 0.5 will round in a direction chosen by the
	/// implementation, presumably the direction that is fastest.
	/// This includes the possibility that round(x) returns the
	/// same value as roundEven(x) for all values of x.
	using glm::round;

	/// Returns a value equal to the nearest integer to x.
	/// A fractional part of 0.5 will round toward the nearest even
	/// integer. (Both 3.5 and 4.5 for x will return 4.0.)
	using glm::roundEven;

	/// Returns a value equal to the nearest integer
	/// that is greater than or equal to x.
	using glm::ceil;

	/// Return x - floor(x).
	using glm::fract;

	/// Modulus. Returns x - y * floor(x / y)
	/// for each component in x using the floating point value y.
	using glm::mod;

	/// Returns the fractional part of x and sets i to the integer
	/// part (as a whole number floating point value). Both the
	/// return value and the output parameter will have the same
	/// sign as x.
	using glm::modf;

	/// Returns y if y < x; otherwise, it returns x.
	using glm::min;

	/// Returns y if x < y; otherwise, it returns x.
	using glm::max;

	/// Returns min(max(x, minVal), maxVal) for each component in x
	/// using the floating-point values minVal and maxVal.
	using glm::clamp;

	// Linear interpolation.
	using glm::mix;

	/// Returns 0.0 if x < edge, otherwise it returns 1.0 for each component of a genType.
	using glm::step;

	/// Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and
	/// performs smooth Hermite interpolation between 0 and 1
	/// when edge0 < x < edge1. This is useful in cases where
	/// you would want a threshold function with a smooth
	/// transition. This is equivalent to:
	/// genType t;
	/// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
	/// return t * t * (3 - 2 * t);
	/// Results are undefined if edge0 >= edge1.
	using glm::smoothstep;

	/// Returns true if x holds a NaN (not a number)
	/// representation in the underlying implementation's set of
	/// floating point representations. Returns false otherwise,
	/// including for implementations with no NaN
	/// representations.
	using glm::isnan;

	/// Returns true if x holds a positive infinity or negative
	/// infinity representation in the underlying implementation's
	/// set of floating point representations. Returns false
	/// otherwise, including for implementations with no infinity
	/// representations.
	using glm::isinf;

	/// Returns a signed integer value representing
	/// the encoding of a floating-point value. The floating-point
	/// value's bit-level representation is preserved.
	using glm::floatBitsToInt;

	/// Returns a unsigned integer value representing
	/// the encoding of a floating-point value. The floatingpoint
	/// value's bit-level representation is preserved.
	using glm::floatBitsToUint;

	/// Returns a floating-point value corresponding to a signed
	/// integer encoding of a floating-point value.
	/// If an inf or NaN is passed in, it will not signal, and the
	/// resulting floating point value is unspecified. Otherwise,
	/// the bit-level representation is preserved.
	using glm::intBitsToFloat;

	/// Returns a floating-point value corresponding to a
	/// unsigned integer encoding of a floating-point value.
	/// If an inf or NaN is passed in, it will not signal, and the
	/// resulting floating point value is unspecified. Otherwise,
	/// the bit-level representation is preserved.
	using glm::uintBitsToFloat;

	/// Computes and returns a * b + c.
	using glm::fma;

	/// Splits x into a floating-point significand in the range
	/// [0.5, 1.0) and an integral exponent of two, such that:
	/// x = significand * exp(2, exponent)
	using glm::frexp;

	/// Builds a floating-point number from x and the
	/// corresponding integral exponent of two in exp, returning:
	/// significand * exp(2, exponent)
	using glm::ldexp;


	// Integer

	/// Adds 32-bit unsigned integer x and y, returning the sum
	/// modulo pow(2, 32). The value carry is set to 0 if the sum was
	/// less than pow(2, 32), or to 1 otherwise.
	using glm::uaddCarry;

	/// Subtracts the 32-bit unsigned integer y from x, returning
	/// the difference if non-negative, or pow(2, 32) plus the difference
	/// otherwise. The value borrow is set to 0 if x >= y, or to 1 otherwise.
	using glm::usubBorrow;

	/// Multiplies 32-bit integers x and y, producing a 64-bit
	/// result. The 32 least-significant bits are returned in lsb.
	/// The 32 most-significant bits are returned in msb.
	using glm::umulExtended;

	/// Multiplies 32-bit integers x and y, producing a 64-bit
	/// result. The 32 least-significant bits are returned in lsb.
	/// The 32 most-significant bits are returned in msb.
	using glm::imulExtended;

	/// Extracts bits [offset, offset + bits - 1] from value,
	/// returning them in the least significant bits of the result.
	/// For unsigned data types, the most significant bits of the
	/// result will be set to zero. For signed data types, the
	/// most significant bits will be set to the value of bit offset + base - 1.
	using glm::bitfieldExtract;

	/// Returns the insertion the bits least-significant bits of insert into base.
	using glm::bitfieldInsert;

	/// Returns the reversal of the bits of value.
	/// The bit numbered n of the result will be taken from bit (bits - 1) - n of value,
	/// where bits is the total number of bits used to represent value.
	using glm::bitfieldReverse;

	// Returns the number of bits set to 1 in the binary representation of value.
	using glm::bitCount;

	/// Returns the bit number of the least significant bit set to
	/// 1 in the binary representation of value.
	/// If value is zero, -1 will be returned.
	using glm::findLSB;

	/// Returns the bit number of the most significant bit in the binary representation of value.
	/// For positive integers, the result will be the bit number of the most significant bit set to 1.
	/// For negative integers, the result will be the bit number of the most significant
	/// bit set to 0. For a value of zero or negative one, -1 will be returned.
	using glm::findMSB;

	// Trigonometric

	/// Converts degrees to radians and returns the result.
	using glm::radians;

	/// Converts radians to degrees and returns the result.
	using glm::degrees;

	/// The standard trigonometric sine function.
	/// The values returned by this function will range from [-1, 1].
	using glm::sin;

	/// The standard trigonometric cosine function.
	/// The values returned by this function will range from [-1, 1].
	using glm::cos;

	/// The standard trigonometric tangent function.
	using glm::tan;

	/// Arc sine. Returns an angle whose sine is x.
	/// The range of values returned by this function is [-PI/2, PI/2].
	/// Results are undefined if |x| > 1.
	using glm::asin;

	/// Arc cosine. Returns an angle whose cosine is x.
	/// The range of values returned by this function is [0, PI].
	/// Results are undefined if |x| > 1.
	using glm::acos;

	/// Arc tangent. Returns an angle whose tangent is y/x.
	/// The signs of x and y are used to determine what
	/// quadrant the angle is in. The range of values returned
	/// by this function is [-PI, PI]. Results are undefined
	/// if x and y are both 0.
	using glm::atan;

	/// Returns the hyperbolic sine function, (exp(x) - exp(-x)) / 2
	using glm::sinh;

	/// Returns the hyperbolic cosine function, (exp(x) + exp(-x)) / 2
	using glm::cosh;

	/// Returns the hyperbolic tangent function, sinh(angle) / cosh(angle)
	using glm::tanh;

	/// Arc hyperbolic sine; returns the inverse of sinh.
	using glm::asinh;

	/// Arc hyperbolic cosine; returns the non-negative inverse
	using glm::acosh;

	/// Arc hyperbolic tangent; returns the inverse of tanh.
	/// Results are undefined if abs(x) >= 1.
	using glm::atanh;

	// Exp

	/// Returns 'base' raised to the power 'exponent'.
	using glm::pow;

	/// Returns the natural exponentiation of v, i.e., e^v.
	using glm::exp;

	/// Returns the natural logarithm of v, i.e.,
	/// returns the value y which satisfies the equation x = e^y.
	/// Results are undefined if v <= 0.
	using glm::log;

	/// Returns 2 raised to the v power.
	using glm::exp2;

	/// Returns the base 2 log of x, i.e., returns the value y,
	/// which satisfies the equation x = 2 ^ y.
	using glm::log2;

	/// Returns the positive square root of v.
	using glm::sqrt;

	/// Returns the reciprocal of the positive square root of v.
	using glm::inversesqrt;

	// Constants

	/// Return 0.
	using glm::zero;

	/// Return 1.
	using glm::one;

	/// Return pi * 2.
	using glm::two_pi;

	/// Return unit-circle circumference, or pi * 2.
	//using glm::tau;

	/// Return square root of pi.
	using glm::root_pi;

	/// Return pi / 2.
	using glm::half_pi;

	/// Return pi / 2 * 3.
	using glm::three_over_two_pi;

	/// Return pi / 4.
	using glm::quarter_pi;

	/// Return 1 / pi.
	using glm::one_over_pi;

	/// Return 1 / (pi * 2).
	using glm::one_over_two_pi;

	/// Return 2 / pi.
	using glm::two_over_pi;

	/// Return 4 / pi.
	using glm::four_over_pi;

	/// Return 2 / sqrt(pi).
	using glm::two_over_root_pi;

	/// Return 1 / sqrt(2).
	using glm::one_over_root_two;

	/// Return sqrt(pi / 2).
	using glm::root_half_pi;

	/// Return sqrt(2 * pi).
	using glm::root_two_pi;

	/// Return sqrt(ln(4)).
	using glm::root_ln_four;

	/// Return e constant.
	using glm::e;

	/// Return Euler's constant.
	using glm::euler;

	/// Return sqrt(2).
	using glm::root_two;

	/// Return sqrt(3).
	using glm::root_three;

	/// Return sqrt(5).
	using glm::root_five;

	/// Return ln(2).
	using glm::ln_two;

	/// Return ln(10).
	using glm::ln_ten;

	/// Return ln(ln(2)).
	using glm::ln_ln_two;

	/// Return 1 / 3.
	using glm::third;

	/// Return 2 / 3.
	using glm::two_thirds;

	/// Return the golden ratio constant.
	using glm::golden_ratio;
}