module;

#include <glm/packing.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/round.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/bitfield.hpp>
#include <glm/gtc/ulp.hpp>

export module glm.util;

export namespace glm
{
	// Packing

	/// First, converts the normalized floating-point value v into a 8-bit integer value.
	/// Then, the results are packed into the returned 8-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm1x8:	round(clamp(c, 0, +1) * 255.0)
	using glm::packUnorm1x8;

	/// Convert a single 8-bit integer to a normalized floating-point value.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackUnorm4x8: f / 255.0
	using glm::unpackUnorm1x8;


	/// First, converts each component of the normalized floating-point value v into 8-bit integer values.
	/// Then, the results are packed into the returned 16-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm2x8:	round(clamp(c, 0, +1) * 255.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packUnorm2x8;

	/// First, unpacks a single 16-bit unsigned integer p into a pair of 8-bit unsigned integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned two-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackUnorm4x8: f / 255.0
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackUnorm2x8;


	/// First, converts the normalized floating-point value v into 8-bit integer value.
	/// Then, the results are packed into the returned 8-bit unsigned integer.
	///
	/// The conversion to fixed point is done as follows:
	/// packSnorm1x8:	round(clamp(s, -1, +1) * 127.0)
	using glm::packSnorm1x8;

	/// First, unpacks a single 8-bit unsigned integer p into a single 8-bit signed integers.
	/// Then, the value is converted to a normalized floating-point value to generate the returned scalar.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm1x8: clamp(f / 127.0, -1, +1)
	using glm::unpackSnorm1x8;


	/// First, converts each component of the normalized floating-point value v into 8-bit integer values.
	/// Then, the results are packed into the returned 16-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packSnorm2x8:	round(clamp(c, -1, +1) * 127.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packSnorm2x8;

	/// First, unpacks a single 16-bit unsigned integer p into a pair of 8-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned two-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm2x8: clamp(f / 127.0, -1, +1)
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackSnorm2x8;


	// First, converts the normalized floating-point value v into a 16-bit integer value.
	/// Then, the results are packed into the returned 16-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm1x16:	round(clamp(c, 0, +1) * 65535.0)
	using glm::packUnorm1x16;

	/// First, unpacks a single 16-bit unsigned integer p into a of 16-bit unsigned integers.
	/// Then, the value is converted to a normalized floating-point value to generate the returned scalar.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackUnorm1x16: f / 65535.0
	using glm::unpackUnorm1x16;


	/// First, converts each component of the normalized floating-point value v into 16-bit integer values.
	/// Then, the results are packed into the returned 64-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm4x16:	round(clamp(c, 0, +1) * 65535.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packUnorm4x16;

	/// First, unpacks a single 64-bit unsigned integer p into four 16-bit unsigned integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackUnormx4x16: f / 65535.0
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackUnorm4x16;


	/// First, converts the normalized floating-point value v into 16-bit integer value.
	/// Then, the results are packed into the returned 16-bit unsigned integer.
	///
	/// The conversion to fixed point is done as follows:
	/// packSnorm1x8:	round(clamp(s, -1, +1) * 32767.0)
	using glm::packSnorm1x16;

	/// First, unpacks a single 16-bit unsigned integer p into a single 16-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned scalar.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm1x16: clamp(f / 32767.0, -1, +1)
	using glm::unpackSnorm1x16;


	/// First, converts each component of the normalized floating-point value v into 16-bit integer values.
	/// Then, the results are packed into the returned 64-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packSnorm2x8:	round(clamp(c, -1, +1) * 32767.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packSnorm4x16;

	/// First, unpacks a single 64-bit unsigned integer p into four 16-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm4x16: clamp(f / 32767.0, -1, +1)
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackSnorm4x16;


	/// Returns an unsigned integer obtained by converting the components of a floating-point scalar
	/// to the 16-bit floating-point representation found in the OpenGL Specification,
	/// and then packing this 16-bit value into a 16-bit unsigned integer.
	using glm::packHalf1x16;

	/// Returns a floating-point scalar with components obtained by unpacking a 16-bit unsigned integer into a 16-bit value,
	/// interpreted as a 16-bit floating-point number according to the OpenGL Specification,
	/// and converting it to 32-bit floating-point values.
	using glm::unpackHalf1x16;


	/// Returns an unsigned integer obtained by converting the components of a four-component floating-point vector
	/// to the 16-bit floating-point representation found in the OpenGL Specification,
	/// and then packing these four 16-bit values into a 64-bit unsigned integer.
	/// The first vector component specifies the 16 least-significant bits of the result;
	/// the forth component specifies the 16 most-significant bits.
	using glm::packHalf4x16;

	/// Returns a four-component floating-point vector with components obtained by unpacking a 64-bit unsigned integer into four 16-bit values,
	/// interpreting those values as 16-bit floating-point numbers according to the OpenGL Specification,
	/// and converting them to 32-bit floating-point values.
	/// The first component of the vector is obtained from the 16 least-significant bits of v;
	/// the forth component is obtained from the 16 most-significant bits of v.
	using glm::unpackHalf4x16;


	/// Returns an unsigned integer obtained by converting the components of a four-component signed integer vector
	/// to the 10-10-10-2-bit signed integer representation found in the OpenGL Specification,
	/// and then packing these four values into a 32-bit unsigned integer.
	/// The first vector component specifies the 10 least-significant bits of the result;
	/// the forth component specifies the 2 most-significant bits.
	using glm::packI3x10_1x2;

	/// Unpacks a single 32-bit unsigned integer p into three 10-bit and one 2-bit signed integers.
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackI3x10_1x2;


	/// Returns an unsigned integer obtained by converting the components of a four-component unsigned integer vector
	/// to the 10-10-10-2-bit unsigned integer representation found in the OpenGL Specification,
	/// and then packing these four values into a 32-bit unsigned integer.
	/// The first vector component specifies the 10 least-significant bits of the result;
	/// the forth component specifies the 2 most-significant bits.
	using glm::packU3x10_1x2;

	/// Unpacks a single 32-bit unsigned integer p into three 10-bit and one 2-bit unsigned integers.
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackU3x10_1x2;


	/// First, converts the first three components of the normalized floating-point value v into 10-bit signed integer values.
	/// Then, converts the forth component of the normalized floating-point value v into 2-bit signed integer values.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packSnorm3x10_1x2(xyz):	round(clamp(c, -1, +1) * 511.0)
	/// packSnorm3x10_1x2(w):	round(clamp(c, -1, +1) * 1.0)
	///
	/// The first vector component specifies the 10 least-significant bits of the result;
	/// the forth component specifies the 2 most-significant bits.
	using glm::packSnorm3x10_1x2;

	/// First, unpacks a single 32-bit unsigned integer p into four 16-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm3x10_1x2(xyz): clamp(f / 511.0, -1, +1)
	/// unpackSnorm3x10_1x2(w): clamp(f / 511.0, -1, +1)
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackSnorm3x10_1x2;


	/// First, converts the first three components of the normalized floating-point value v into 10-bit unsigned integer values.
	/// Then, converts the forth component of the normalized floating-point value v into 2-bit signed uninteger values.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm3x10_1x2(xyz):	round(clamp(c, 0, +1) * 1023.0)
	/// packUnorm3x10_1x2(w):	round(clamp(c, 0, +1) * 3.0)
	///
	/// The first vector component specifies the 10 least-significant bits of the result;
	/// the forth component specifies the 2 most-significant bits.
	using glm::packUnorm3x10_1x2;

	/// First, unpacks a single 32-bit unsigned integer p into four 16-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm3x10_1x2(xyz): clamp(f / 1023.0, 0, +1)
	/// unpackSnorm3x10_1x2(w): clamp(f / 3.0, 0, +1)
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackUnorm3x10_1x2;


	/// First, converts the first two components of the normalized floating-point value v into 11-bit signless floating-point values.
	/// Then, converts the third component of the normalized floating-point value v into a 10-bit signless floating-point value.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The first vector component specifies the 11 least-significant bits of the result;
	/// the last component specifies the 10 most-significant bits.
	using glm::packF2x11_1x10;

	/// First, unpacks a single 32-bit unsigned integer p into two 11-bit signless floating-point values and one 10-bit signless floating-point value .
	/// Then, each component is converted to a normalized floating-point value to generate the returned three-component vector.
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackF2x11_1x10;


	/// First, converts the first two components of the normalized floating-point value v into 11-bit signless floating-point values.
	/// Then, converts the third component of the normalized floating-point value v into a 10-bit signless floating-point value.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The first vector component specifies the 11 least-significant bits of the result;
	/// the last component specifies the 10 most-significant bits.
	///
	/// packF3x9_E1x5 allows encoding into RGBE / RGB9E5 format
	using glm::packF3x9_E1x5;

	/// First, unpacks a single 32-bit unsigned integer p into two 11-bit signless floating-point values and one 10-bit signless floating-point value .
	/// Then, each component is converted to a normalized floating-point value to generate the returned three-component vector.
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	///
	/// unpackF3x9_E1x5 allows decoding RGBE / RGB9E5 data
	using glm::unpackF3x9_E1x5;


	/// Returns an unsigned integer vector obtained by converting the components of a floating-point vector
	/// to the 16-bit floating-point representation found in the OpenGL Specification.
	/// The first vector component specifies the 16 least-significant bits of the result;
	/// the forth component specifies the 16 most-significant bits.
	using glm::packRGBM;

	/// Returns a floating-point vector with components obtained by reinterpreting an integer vector as 16-bit floating-point numbers and converting them to 32-bit floating-point values.
	/// The first component of the vector is obtained from the 16 least-significant bits of v;
	/// the forth component is obtained from the 16 most-significant bits of v.
	using glm::unpackRGBM;

	/// Returns an unsigned integer vector obtained by converting the components of a floating-point vector
	/// to the 16-bit floating-point representation found in the OpenGL Specification.
	/// The first vector component specifies the 16 least-significant bits of the result;
	/// the forth component specifies the 16 most-significant bits.
	using glm::packHalf;

	/// Returns a floating-point vector with components obtained by reinterpreting an integer vector as 16-bit floating-point numbers and converting them to 32-bit floating-point values.
	/// The first component of the vector is obtained from the 16 least-significant bits of v;
	/// the forth component is obtained from the 16 most-significant bits of v.
	using glm::unpackHalf;

	/// Convert each component of the normalized floating-point vector into unsigned integer values.
	using glm::packUnorm;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackUnorm;

	/// Convert each component of the normalized floating-point vector into signed integer values.
	using glm::packSnorm;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackSnorm;

	/// Convert each component of the normalized floating-point vector into unsigned integer values.
	using glm::packUnorm2x4;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackUnorm2x4;


	/// Convert each component of the normalized floating-point vector into unsigned integer values.
	using glm::packUnorm4x4;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackUnorm4x4;


	/// Convert each component of the normalized floating-point vector into unsigned integer values.
	using glm::packUnorm1x5_1x6_1x5;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackUnorm1x5_1x6_1x5;


	/// Convert each component of the normalized floating-point vector into unsigned integer values.
	using glm::packUnorm3x5_1x1;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackUnorm3x5_1x1;


	/// Convert each component of the normalized floating-point vector into unsigned integer values.
	using glm::packUnorm2x3_1x2;

	/// Convert a packed integer to a normalized floating-point vector.
	using glm::unpackUnorm2x3_1x2;


	/// Convert each component from an integer vector into a packed integer.
	using glm::packInt2x8;

	/// Convert a packed integer into an integer vector.
	using glm::unpackInt2x8;


	/// Convert each component from an integer vector into a packed unsigned integer.
	using glm::packUint2x8;

	/// Convert a packed integer into an integer vector.
	using glm::unpackUint2x8;


	/// Convert each component from an integer vector into a packed integer.
	using glm::packInt4x8;

	/// Convert a packed integer into an integer vector.
	using glm::unpackInt4x8;


	/// Convert each component from an integer vector into a packed unsigned integer.
	using glm::packUint4x8;

	/// Convert a packed integer into an integer vector.
	using glm::unpackUint4x8;


	/// Convert each component from an integer vector into a packed integer.
	using glm::packInt2x16;

	/// Convert a packed integer into an integer vector.
	using glm::unpackInt2x16;


	/// Convert each component from an integer vector into a packed integer.
	using glm::packInt4x16;

	/// Convert a packed integer into an integer vector.
	using glm::unpackInt4x16;


	/// Convert each component from an integer vector into a packed unsigned integer.
	using glm::packUint2x16;

	/// Convert a packed integer into an integer vector.
	using glm::unpackUint2x16;


	/// Convert each component from an integer vector into a packed unsigned integer.
	using glm::packUint4x16;

	/// Convert a packed integer into an integer vector.
	using glm::unpackUint4x16;


	/// Convert each component from an integer vector into a packed integer.
	using glm::packInt2x32;

	/// Convert a packed integer into an integer vector.
	using glm::unpackInt2x32;


	/// Convert each component from an integer vector into a packed unsigned integer.
	using glm::packUint2x32;

	/// Convert a packed integer into an integer vector.
	using glm::unpackUint2x32;


	/// First, converts each component of the normalized floating-point value v into 8- or 16-bit integer values.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packUnorm2x16;

	/// First, converts each component of the normalized floating-point value v into 8- or 16-bit integer values.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packSnorm2x16: round(clamp(v, -1, +1) * 32767.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packSnorm2x16;

	/// First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, four 8-bit unsigned integers, or four 8-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackUnorm2x16: f / 65535.0
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackUnorm2x16;

	/// First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, four 8-bit unsigned integers, or four 8-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm2x16: clamp(f / 32767.0, -1, +1)
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackSnorm2x16;


	/// Returns a double-qualifier value obtained by packing the components of v into a 64-bit value.
	/// If an IEEE 754 Inf or NaN is created, it will not signal, and the resulting floating point value is unspecified.
	/// Otherwise, the bit- level representation of v is preserved.
	/// The first vector component specifies the 32 least significant bits;
	/// the second component specifies the 32 most significant bits.
	using glm::packDouble2x32;

	/// Returns a two-component unsigned integer vector representation of v.
	/// The bit-level representation of v is preserved.
	/// The first component of the vector contains the 32 least significant bits of the double;
	/// the second component consists the 32 most significant bits.
	using glm::unpackDouble2x32;


	/// Returns an unsigned integer obtained by converting the components of a two-component floating-point vector
	/// to the 16-bit floating-point representation found in the OpenGL Specification,
	/// and then packing these two 16- bit integers into a 32-bit unsigned integer.
	/// The first vector component specifies the 16 least-significant bits of the result;
	/// the second component specifies the 16 most-significant bits.
	using glm::packHalf2x16;

	/// Returns a two-component floating-point vector with components obtained by unpacking a 32-bit unsigned integer into a pair of 16-bit values,
	/// interpreting those values as 16-bit floating-point numbers according to the OpenGL Specification,
	/// and converting them to 32-bit floating-point values.
	/// The first component of the vector is obtained from the 16 least-significant bits of v;
	/// the second component is obtained from the 16 most-significant bits of v.
	using glm::unpackHalf2x16;

	// First, converts each component of the normalized floating-point value v into 8- or 16-bit integer values.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packUnorm4x8:	round(clamp(c, 0, +1) * 255.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packUnorm4x8;

	// First, converts each component of the normalized floating-point value v into 8- or 16-bit integer values.
	/// Then, the results are packed into the returned 32-bit unsigned integer.
	///
	/// The conversion for component c of v to fixed point is done as follows:
	/// packSnorm4x8:	round(clamp(c, -1, +1) * 127.0)
	///
	/// The first component of the vector will be written to the least significant bits of the output;
	/// the last component will be written to the most significant bits.
	using glm::packSnorm4x8;

	/// First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, four 8-bit unsigned integers, or four 8-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackUnorm4x8: f / 255.0
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackUnorm4x8;

	/// First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, four 8-bit unsigned integers, or four 8-bit signed integers.
	/// Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
	///
	/// The conversion for unpacked fixed-point value f to floating point is done as follows:
	/// unpackSnorm4x8: clamp(f / 127.0, -1, +1)
	///
	/// The first component of the returned vector will be extracted from the least significant bits of the input;
	/// the last component will be extracted from the most significant bits.
	using glm::unpackSnorm4x8;



	// Noise

	/// Classic perlin noise.
	using glm::perlin;

	/// Simplex noise.
	using glm::simplex;

	// Random

	/// Generate random numbers in the interval [Min, Max], according a linear distribution
	using glm::linearRand;
	
	/// Generate random numbers in the interval [Min, Max], according a gaussian distribution
	using glm::gaussRand;

	/// Generate a random 2D vector which coordinates are regularly distributed on a circle of a given radius
	using glm::circularRand;

	/// Generate a random 3D vector which coordinates are regularly distributed on a sphere of a given radius
	using glm::sphericalRand;

	/// Generate a random 2D vector which coordinates are regularly distributed within the area of a disk of a given radius
	using glm::diskRand;

	/// Generate a random 3D vector which coordinates are regularly distributed within the volume of a ball of a given radius
	using glm::ballRand;

	// Round

	/// Return the power of two number which value is just higher the input value,
	/// round up to a power of two.
	using glm::ceilPowerOfTwo;

	/// Return the power of two number which value is just lower the input value,
	/// round down to a power of two.
	using glm::floorPowerOfTwo;

	/// Return the power of two number which value is the closet to the input value.
	using glm::roundPowerOfTwo;

	/// Higher multiple number of Source.
	using glm::ceilMultiple;

	/// Lower multiple number of Source.
	using glm::floorMultiple;

	/// Lower multiple number of Source.
	using glm::roundMultiple;

	// Epsilon

	/// Returns the component-wise comparison of |x - y| < epsilon.
	/// True if this expression is satisfied.
	using glm::epsilonEqual;

	/// Returns the component-wise comparison of |x - y| < epsilon.
	/// True if this expression is not satisfied.
	using glm::epsilonNotEqual;



	// Color space

	/// Convert a linear color to sRGB color using a standard gamma correction.
	/// IEC 61966-2-1:1999 / Rec. 709 specification https://www.w3.org/Graphics/Color/srgb
	using glm::convertLinearToSRGB;

	/// Convert a sRGB color to linear color using a standard gamma correction.
	/// IEC 61966-2-1:1999 / Rec. 709 specification https://www.w3.org/Graphics/Color/srgb
	using glm::convertSRGBToLinear;



	// Bitfield

	/// Build a mask of 'count' bits
	using glm::mask;

	/// Rotate all bits to the right. All the bits dropped in the right side are inserted back on the left side.
	using glm::bitfieldRotateRight;

	/// Rotate all bits to the left. All the bits dropped in the left side are inserted back on the right side.
	using glm::bitfieldRotateLeft;

	/// Set to 1 a range of bits.
	using glm::bitfieldFillOne;

	/// Set to 0 a range of bits.
	using glm::bitfieldFillZero;

	/// Interleaves the bits of x and y.
	/// The first bit is the first bit of x followed by the first bit of y.
	/// The other bits are interleaved following the previous sequence.
	using glm::bitfieldInterleave;

	/// Deinterleaves the bits of x.
	using glm::bitfieldDeinterleave;

	// upl

	/// Return the next ULP value(s) after the input value(s).
	using glm::next_float;

	/// Return the previous ULP value(s) before the input value(s).
	using glm::prev_float;

	/// Return the distance in the number of ULP between 2 single-precision floating-point scalars.
	using glm::float_distance;
}