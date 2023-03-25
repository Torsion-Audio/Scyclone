#ifndef _RNBO_MATH_H_
#define _RNBO_MATH_H_

#include "RNBO_Std.h"
#include "RNBO_CMath.h"

#ifndef RNBO_NOSTDLIB
#include <cmath>
#include <limits>
#endif // RNBO_NOSTDLIB

#include "RNBO_MathFast.h"

namespace RNBO_Math {
	constexpr static const RNBO::number E		= RNBO::number(2.7182818284590452354);
	constexpr static const RNBO::number LN10	= RNBO::number(2.302585092994046);
	constexpr static const RNBO::number LN2		= RNBO::number(0.6931471805599453);
	constexpr static const RNBO::number LOG10E	= RNBO::number(0.4342944819032518);
	constexpr static const RNBO::number LOG2E	= RNBO::number(1.4426950408889634);
#if __has_builtin(__builtin_nanf)
	constexpr static const RNBO::number NaN		= RNBO::number(__builtin_nanf("0x7fc00000"));
#elif !defined(RNBO_NOSTDLIB)
	constexpr static const float NaN			= std::numeric_limits<float>::quiet_NaN();
#elif defined(NAN)
	constexpr static const RNBO::number NaN		= RNBO::number(NAN);
#else
	static const RNBO::number NaN				= RNBO::number(0.) / RNBO::number(0.);
#endif

#ifdef PI
#undef PI
#endif

	constexpr static const RNBO::number PI		= RNBO::number(3.1415926535897932);
	constexpr static const RNBO::number TWOPI	= RNBO::number(6.2831853071795865);
	constexpr static const RNBO::number SQRT1_2	= RNBO::number(0.7071067811865476);
	constexpr static const RNBO::number SQRT2	= RNBO::number(1.4142135623730951);

	inline int32_t rnbo_imul(int32_t x, int32_t y) { return x * y; }

	template <typename T>
	inline int rnbo_sign(T x) { return x > 0 ? 1 : x < 0 ? -1 : x; }

	template <typename T>
	inline int rnbo_isnan(T x) {
#ifdef RNBO_NOSTDLIB
		return x != x;
#else
		return std::isnan(x);
#endif
	}

	template <typename T>
	inline int rnbo_isfinite(T x) {
	#ifdef RNBO_NOSTDLIB
		const bool isinf = !rnbo_isnan<T>(x) && rnbo_isnan<T>(x - x);
		return !isinf;
	#else
		return std::isfinite(x);
	#endif
	}

	inline int rnbo_isnan(int /* x */) {
		return false;
	}

	constexpr RNBO::number rnbo_number_max() {
#ifndef RNBO_NOSTDLIB
		//funny syntax is to work around windows.h max macro conflict
		return (std::numeric_limits<RNBO::number>::max)();
#else
#ifdef RNBO_USE_FLOAT32

#if defined(__FLT_MAX__)
		return __FLT_MAX__;
#elif defined(FLT_MAX)
		return FLT_MAX;
#else
		return 3.402823466e+38f;
#endif

#else

#if defined(__DBL_MAX__)
		return __DBL_MAX__;
#elif defined(DBL_MAX)
		return DBL_MAX;
#else
		return 1.7976931348623158e+308;
#endif

#endif
#endif
	}

	template <typename T> inline RNBO::number rnbo_acos(T x) { return ::acos(x); }
	template <> inline RNBO::number rnbo_acos<float>(float x) { return ::acosf(x); }

	template <typename T> inline RNBO::number rnbo_acosh(T x) { return ::acosh(x); }
	template <> inline RNBO::number rnbo_acosh<float>(float x) { return ::acoshf(x); }

	template <typename T> inline RNBO::number rnbo_asin(T x) { return ::asin(x); }
	template <> inline RNBO::number rnbo_asin<float>(float x) { return ::asinf(x); }

	template <typename T> inline RNBO::number rnbo_asinh(T x) { return ::asinh(x); }
	template <> inline RNBO::number rnbo_asinh<float>(float x) { return ::asinhf(x); }

	template <typename T> inline RNBO::number rnbo_atan(T x) { return ::atan(x); }
	template <> inline RNBO::number rnbo_atan<float>(float x) { return ::atanf(x); }

	template <typename T, typename U> inline RNBO::number rnbo_atan2(T y, U x) { return ::atan2(y, x); }
	template <> inline RNBO::number rnbo_atan2<float, float>(float y, float x) { return ::atan2f(y, x); }

	template <typename T> inline RNBO::number rnbo_atanh(T x) { return ::atanh(x); }
	template <> inline RNBO::number rnbo_atanh<float>(float x) { return ::atanhf(x); }

	template <typename T> inline RNBO::number rnbo_cbrt(T x) { return ::cbrt(x); }
	template <> inline RNBO::number rnbo_cbrt<float>(float x) { return ::cbrtf(x); }

	template <typename T> inline RNBO::number rnbo_ceil(T x) { return ::ceil(x); }
	template <> inline RNBO::number rnbo_ceil<float>(float x) { return ::ceilf(x); }

	template <typename T> inline RNBO::number rnbo_cos(T x) { return ::cos(x); }
	template <> inline RNBO::number rnbo_cos<float>(float x) { return ::cosf(x); }

	template <typename T> inline RNBO::number rnbo_cosh(T x) { return ::cosh(x); }
	template <> inline RNBO::number rnbo_cosh<float>(float x) { return ::coshf(x); }

	template <typename T> inline RNBO::number rnbo_exp(T x) { return ::exp(x); }
	template <> inline RNBO::number rnbo_exp<float>(float x) { return ::expf(x); }

	template <typename T> inline T rnbo_abs(T x) { return abs(x); }
	template <> inline float rnbo_abs<float>(float x) { return ::fabsf(x); }
	template <> inline double rnbo_abs<double>(double x) { return ::fabs(x); }
	template <> inline bool rnbo_abs<bool>(bool x) { return x; }

	template <typename T> inline T rnbo_fabs(T x) { return ::fabs(x); }
	template <> inline float rnbo_fabs<float>(float x) { return ::fabsf(x); }
	template <> inline double rnbo_fabs<double>(double x) { return ::fabs(x); }

	template <typename T> inline RNBO::number rnbo_expm1(T x) { return ::expm1(x); }
	template <> inline RNBO::number rnbo_expm1<float>(float x) { return ::expm1f(x); }

	template <typename T> inline RNBO::number rnbo_floor(T x) { return ::floor(x); }
	template <> inline RNBO::number rnbo_floor<float>(float x) { return ::floorf(x); }

	template <typename T>
	inline T rnbo_fround(T x) { // not 100% compliant, but it will do for now
		T t;
		if (x >= T(0.0)) {
			t = rnbo_ceil(x);
			if (t - x > T(0.5)) t -= T(1.0);
			return t;
		} else {
			t = rnbo_ceil(-x);
			if (t + x > T(0.5)) t -= T(1.0);
			return -t;
		}
	}

	template <typename T> inline RNBO::Int rnbo_trunc(T x)  { return (RNBO::Int)x; }

	template <typename T> inline RNBO::number rnbo_log2(T x) { return ::log2(x); }
	template <> inline RNBO::number rnbo_log2<float>(float x) { return ::log2f(x); }

	template <typename T> inline RNBO::number rnbo_log(T x) { return ::log(x); }
	template <> inline RNBO::number rnbo_log<float>(float x) { return ::logf(x); }

	template <typename T> inline RNBO::number rnbo_log10(T x) { return ::log10(x); }
	template <> inline RNBO::number rnbo_log10<float>(float x) { return ::log10f(x); }

	template <typename T> inline RNBO::number rnbo_log1p(T x) { return ::log1p(x); }
	template <> inline RNBO::number rnbo_log1p<float>(float x) { return ::log1pf(x); }

	template <typename T, typename U> inline RNBO::number rnbo_pow(T x, U y) { return ::pow(x, y); }
	template <> inline RNBO::number rnbo_pow<float, float>(float x, float y) { return ::powf(x, y); }

	template <typename T> inline RNBO::number rnbo_sin(T x) { return ::sin(x); }
	template <> inline RNBO::number rnbo_sin<float>(float x) { return ::sinf(x); }

	template <typename T> inline RNBO::number rnbo_sinh(T x) { return ::sinh(x); }
	template <> inline RNBO::number rnbo_sinh<float>(float x) { return ::sinhf(x); }

	template <typename T> inline RNBO::number rnbo_sqrt(T x) { return ::sqrt(x); }
	template <> inline RNBO::number rnbo_sqrt<float>(float x) { return ::sqrtf(x); }

	template <typename T> inline RNBO::number rnbo_tan(T x) { return ::tan(x); }
	template <> inline RNBO::number rnbo_tan<float>(float x) { return ::tanf(x); }

	template <typename T> inline RNBO::number rnbo_tanh(T x) { return ::tanh(x); }
	template <> inline RNBO::number rnbo_tanh<float>(float x) { return ::tanhf(x); }
}

// following ifdef prevents a conflict with the definition of log2 in genlib_ops.h
// TODO: I'm sure we can come up with a cleaner way for genlib and RNBO to play nicely together
#ifndef GENLIB_OPS_H

namespace RNBO {
	using RNBO_Math::PI;
	using RNBO_Math::TWOPI;
	using RNBO_Math::NaN;

	using RNBO_Math::rnbo_fround;
	using RNBO_Math::rnbo_imul;
	using RNBO_Math::rnbo_sign;
	using RNBO_Math::rnbo_isnan;
	using RNBO_Math::rnbo_number_max;

	using RNBO_Math::rnbo_abs;
	using RNBO_Math::rnbo_acos;
	using RNBO_Math::rnbo_asin;
	using RNBO_Math::rnbo_atan;
	using RNBO_Math::rnbo_atan2;
	using RNBO_Math::rnbo_ceil;
	using RNBO_Math::rnbo_cos;
	using RNBO_Math::rnbo_cosh;
	using RNBO_Math::rnbo_exp;
	using RNBO_Math::rnbo_fabs;
	using RNBO_Math::rnbo_floor;
	using RNBO_Math::rnbo_trunc;
	using RNBO_Math::rnbo_log2;
	using RNBO_Math::rnbo_log;
	using RNBO_Math::rnbo_log10;
	using RNBO_Math::rnbo_log1p;
	using RNBO_Math::rnbo_pow;
	using RNBO_Math::rnbo_sin;
	using RNBO_Math::rnbo_sinh;
	using RNBO_Math::rnbo_sqrt;
	using RNBO_Math::rnbo_tan;
	using RNBO_Math::rnbo_tanh;
	using RNBO_Math::rnbo_acosh;
	using RNBO_Math::rnbo_asinh;
	using RNBO_Math::rnbo_atanh;
	using RNBO_Math::rnbo_cbrt;
	using RNBO_Math::rnbo_expm1;
}

#endif // #ifndef GENLIB_OPS

/*
using RNBO_Math::exp2 = ::exp2f;
using RNBO_MATH::expm1 = ::expm1f;
using RNBO_MATH::log2 = ::log2f;
using RNBO_MATH::log1p = ::log1pf;
using RNBO_MATH::logb = ::logbf;
*/

// these shouldn't be necessary for the normal float/double case the the functions
// are properly templated in <cmath>. However, fround, imul, and sign
// are not available in C++ (but are available in JS) and need to be defined.

/*
 // all Math.NNN functions from ECMAScript 5 & 2015
 inline T abs(T x) { return std::abs(x); }
 inline T acos(T x) { return std::acos(x); }
 inline T asin(T x) { return std::asin(x); }
 inline T atan(T x) { return std::atan(x); }
 inline T atan2(T y, T x) { return std::atan2(y, x); }
 inline T ceil(T x) { return std::ceil(x); }
 inline T cos(T x) { return std::cos(x); }
 inline T exp(T x) { return std::exp(x); }
 inline T fabs(T x) { return std::fabs(x); } // not in JS, including for completeness
 inline T floor(T x) { return std::floor(x); }
 inline T log(T x) { return std::log(x); }
 inline T pow(T x, T y) { return std::pow(x, y); }
 inline T round(T x) { return std::round(x); }
 inline T sin(T x) { return std::sin(x); }
 inline T sinh(T x) { return std::sinh(x); }
 inline T sqrt(T x) { return std::sqrt(x); }
 inline T tan(T x) { return std::tan(x); }
 inline T tanh(T x) { return std::tanh(x); }
 inline T trunc(T x) { return std::trunc(x); } // ECMAScript 2015

 inline T acosh(T x) { return std::acosh(x); } // ECMAScript 2015
 inline T asinh(T x) { return std::asinh(x); } // ECMAScript 2015
 inline T atanh(T x) { return std::atanh(x); } // ECMAScript 2015
 inline T cbrt(T x) { return std::cbrt(x); } // ECMAScript 2015
 // clz32 TODO with __builtin? ECMAScript 2015
 inline T cosh(T x) { return std::cosh(x); } // ECMAScript 2015
 inline T expm1(T x) { return std::expm1(x); } // ECMAScript 2015
 inline T fround(T x) { return (T)std::round(x); } // ECMAScript 2015, not in <cmath>
 inline T hypot(T x, T y) { return std::hypot(x, y); } // ECMAScript 2015, different in JS (allows variable-ed arg list)
 inline T imul(T x, T y) { return T(uint32_t(x) * uint32_t(y)); } // ECMAScript 2015, not in <cmath>
 inline T log1p(T x) { return std::log1p(x); } // ECMAScript 2015
 inline T log10(T x) { return std::log10(x); } // ECMAScript 2015
 inline T log2(T x) { return std::log2(x); } // ECMAScript 2015
 inline T max(T x, T y) { return std::fmax(x, y); } // different in JS (allows variable-sized arg list)
 inline T min(T x, T y) { return std::fmin(x, y); } // different in JS (allows variable-sized arg list)
 // T random() { return something, maybe using c++11 rng; } // ECMAScript 2015
 inline int sign(T x) { return x > 0 ? 1 : x < 0 ? -1 : x; } // ECMAScript 2015, not in <cmath>
 */

////////////////////////////////

#ifdef RNBO_USE_APPROXIMATE_MATH

// note that these are all single-precision
namespace RNBO_ApproximateMath { // we might want a separate RNBO_ARMMath header for ARM-specific stuff

	static float rnbo_cos(float x) { return fastcos(x); }
	static float rnbo_cosh(float x) { return fastcosh(x); }
	static float rnbo_sin(float x) { return fastsin(x); }
	static float rnbo_sinh(float x) { return fastsinh(x); }
	static float rnbo_tan(float x) { return fasttan(x); }
	static float rnbo_tanh(float x) { return fasttanh(x); }
	static float rnbo_pow(float x, float y) { return fastpow(x, y); }
	static float rnbo_pow2(float x) { return fastpow2(x); }
	static float rnbo_exp(float x) { return fastexp(x); }
	static float rnbo_log2(float x) { return fastlog2(x); }

	///////////////////////////////////////////////////////

	// from http://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
	static float rnbo_atan2(float y, float x) {
		using namespace std;
		const float coeff_1 = RNBO_Math::PI/4;
		const float coeff_2 = 3*RNBO_Math::PI/4;
		float abs_y = fabs(y)+1e-10; // kludge to prevent 0/0 condition
		float r, angle;
		if (x>=0){
			r = (x - abs_y) / (x + abs_y);
			angle = coeff_1 - coeff_1 * r;
		}else{
			r = (x + abs_y) / (abs_y - x);
			angle = coeff_2 - coeff_1 * r;
		}
		if(y < 0)
			return(-angle); // negate if in quad III or IV
		else
			return(angle);
	}

#if 0 // alternatives
	// http://math.stackexchange.com/questions/107292/rapid-approximation-of-tanhx
	// what else we got?
	inline float rnbo_tanh(float x) {
		return (-.67436811832e-5+(.2468149110712040+(.583691066395175e-1+.3357335044280075e-1*x)*x)*x)/
		(.2464845986383725+(.609347197060491e-1+(.1086202599228572+.2874707922475963e-1*x)*x)*x);
	}

	// Andrew Simper's pow(2, x) aproximation from the music-dsp list
	float fastpow2f(float x) // possible alternative to pow2 above
	{
		int32_t *px = (int32_t*)(&x); // store address of float as long pointer
		const float tx = (x-0.5f) + (3<<22); // temporary value for truncation
		const long  lx = *((int32_t*)&tx) - 0x4b400000; // integer power of 2
		const float dx = x-(float)(lx); // float remainder of power of 2
		x = 1.0f + dx*(0.6960656421638072f + // cubic apporoximation of 2^x
					   dx*(0.224494337302845f +  // for x in the range [0, 1]
						   dx*(0.07944023841053369f)));
		*px += (lx<<23); // add integer power of 2 to exponent
		return x;
	}

	/* ----------------------------------------------------------------------
	 ** Fast approximation to the log2() function.  It uses a two step
	 ** process.  First, it decomposes the floating-point number into
	 ** a fractional component F and an exponent E.  The fraction component
	 ** is used in a polynomial approximation and then the exponent added
	 ** to the result.  A 3rd order polynomial is used and the result
	 ** when computing db20() is accurate to 7.984884e-003 dB.
	 ** http://community.arm.com/thread/6741
	 ** ------------------------------------------------------------------- */
	const float log2f_approx_coeff[4] = {1.23149591368684f, -4.11852516267426f, 6.02197014179219f, -3.13396450166353f};
	float fastlog2f(float X){
		const float *C = &log2f_approx_coeff[0];
		float Y;
		float F;
		int E;
		// This is the approximation to log2()
		F = frexpf(fabsf(X), &E);
		//  Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
		Y = *C++;
		Y *= F;
		Y += (*C++);
		Y *= F;
		Y += (*C++);
		Y *= F;
		Y += (*C++);
		Y += E;
		return(Y);
	}
#endif // alternatives
}

using RNBO_ApproximateMath::rnbo_cos;
using RNBO_ApproximateMath::rnbo_cosh;
using RNBO_ApproximateMath::rnbo_sin;
using RNBO_ApproximateMath::rnbo_sinh;
using RNBO_ApproximateMath::rnbo_tan;
using RNBO_ApproximateMath::rnbo_tanh;
using RNBO_ApproximateMath::rnbo_pow;
using RNBO_ApproximateMath::rnbo_pow2;
using RNBO_ApproximateMath::rnbo_exp;
using RNBO_ApproximateMath::rnbo_log2;

#endif // RNBO_USE_APPROXIMATE_MATH

///////////////////////////////////

#ifdef ARM_CORTEX

#include "arm_math.h"

namespace RNBO_ARMMath {

	static uint32_t r32seed = 33641;

	static void arm_srand32(uint32_t s) {
		r32seed = s;
	}

	/**
	 * generate an unsigned 32bit pseudo-random number using xorshifter algorithm.
	 * "Anyone who considers arithmetical methods of producing random digits is, of course, in a state of sin."
	 * -- John von Neumann.
	 */
	static uint32_t arm_rand32() {
		r32seed ^= r32seed << 13;
		r32seed ^= r32seed >> 17;
		r32seed ^= r32seed << 5;
		return r32seed;
	}

	static float arm_sqrtf(float in) {
		float out;
		arm_sqrt_f32(in, &out);
		return out;
	}

	static float rnbo_sin(float x) { return arm_sin_f32(x); }
	static float sinf(float x) { return arm_sin_f32(x); }
	static float rnbo_cos(float x) { return arm_cos_f32(x); }
	static float cosf(float x) { return arm_cos_f32(x); }
	static float rnbo_sqrt(float x) { return arm_sqrtf(x); }
	static float sqrtf(float x) { return arm_sqrtf(x); }
	static float rand() { return arm_rand32(); }

}

// it may be that the approximate math functions, or even the <cmath> functions
// are faster, this will need to be tested. However, wanted to have these in place.
/*
using RNBO_ARMMath::sin;
using RNBO_ARMMath::sinf;
using RNBO_ARMMath::cos;
using RNBO_ARMMath::cosf;
using RNBO_ARMMath::sqrt;
using RNBO_ARMMath::sqrtf;
using RNBO_ARMMath::rand;
*/
#endif //ARM_CORTEX

///////////////////////////////////

namespace RNBO {

namespace RNBO_Gen {

#ifdef RNBO_USE_FLOAT32

// assumes v is a 32-bit float
#define RNBO_IS_NAN_FLOAT(v)			((v)!=(v))
#define RNBO_FIX_NAN_FLOAT(v)			((v)=RNBO_IS_NAN_FLOAT(v)?0.f:(v))

#ifdef RNBO_NO_DENORM_TEST
#define RNBO_IS_DENORM_FLOAT(v)			(v)
#define RNBO_FIX_DENORM_FLOAT(v)		(v)
#else
#ifdef WIN32
#define __FLT_MIN__ (FLT_MIN)
#endif
#define RNBO_IS_DENORM_FLOAT(v)			((v)!=0.&&fabs(v)<__FLT_MIN__)
#define RNBO_FIX_DENORM_FLOAT(v)		((v)=RNBO_IS_DENORM_FLOAT(v)?0.f:(v))
#endif

#define RNBO_IS_NAN		RNBO_IS_NAN_FLOAT
#define RNBO_FIX_NAN	RNBO_FIX_NAN_FLOAT
#define RNBO_IS_DENORM	RNBO_IS_DENORM_FLOAT
#define RNBO_FIX_DENORM	RNBO_FIX_DENORM_FLOAT

#else // RNBO_USE_FLOAT32

// assumes v is a 64-bit double:
#define RNBO_IS_NAN_DOUBLE(v)			((((reinterpret_cast<uint32_t*>(&(v)))[1])&0x7fe00000)==0x7fe00000)
#define RNBO_FIX_NAN_DOUBLE(v)			((v)=RNBO_IS_NAN_DOUBLE(v)?0.:(v))

#ifdef RNBO_NO_DENORM_TEST
	#define RNBO_IS_DENORM_DOUBLE(v)	(v)
	#define RNBO_FIX_DENORM_DOUBLE(v)	(v)
#else
	#define RNBO_IS_DENORM_DOUBLE(v)	(((((reinterpret_cast<uint32_t*>(&(v)))[1])&0x7fe00000)==0)&&((v)!=0.))
	#define RNBO_FIX_DENORM_DOUBLE(v)	((v)=RNBO_IS_DENORM_DOUBLE(v)?0.f:(v))

#endif // RNBO_USE_FLOAT32

#define RNBO_IS_NAN		RNBO_IS_NAN_DOUBLE
#define RNBO_FIX_NAN	RNBO_FIX_NAN_DOUBLE
#define RNBO_IS_DENORM	RNBO_IS_DENORM_DOUBLE
#define RNBO_FIX_DENORM	RNBO_FIX_DENORM_DOUBLE
#endif

	template <typename T> inline
	T fixnan(T v) { return RNBO_FIX_NAN(v); }

	template <typename T> inline
	T fixdenorm(T v) { return RNBO_FIX_DENORM(v); }

	template <typename T> inline
	T isdenorm(T v) { return RNBO_IS_DENORM(v); }

//	template <typename T> inline
//	T isnan(T v) { return RNBO_IS_NAN(v); }
//
#ifdef RNBO_USE_FLOAT32
	inline float fract (float value) {
		float unused;
		return modff(value, &unused);
	}
#else
	inline double fract (double value) {
		double unused;
		return modf(value, &unused);
	}
#endif

	inline float fastsin(float x) {
		return RNBO_ApproximateMath::fastersinfull(x);
	}

	inline float fastcos(float x) {
		return RNBO_ApproximateMath::fastercosfull(x);
	}

	inline float fastexp(float x) {
		return RNBO_ApproximateMath::fasterexp(x);
	}

	inline float fastpow(float x, float p) {
		return RNBO_ApproximateMath::fasterpow(x, p);
	}

	inline float fasttan(float x) {
		return RNBO_ApproximateMath::fastertanfull(x);
	}

	inline float fastpow2(float x) {
		return RNBO_ApproximateMath::fasterpow2(x);
	}

	inline float fastlog2(float x) {
		return RNBO_ApproximateMath::fasterlog2(x);
	}

	// from http://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
	inline float fasttan2(float y, float x) {
		const float coeff_1 = (float)(RNBO_Math::PI/4);
		const float coeff_2 = 3 * (float)(RNBO_Math::PI/4);
		float abs_y = (float)fabs(y) + (float)1e-10; // kludge to prevent 0/0 condition
		float r, angle;
		if (x >= 0) {
			r = (x - abs_y) / (x + abs_y);
			angle = coeff_1 - coeff_1 * r;
		}
		else {
			r = (x + abs_y) / (abs_y - x);
			angle = coeff_2 - coeff_1 * r;
		}
		if (y < 0) {
			return (-angle); // negate if in quad III or IV
		}
		else {
			return (angle);
		}
	}

	inline float fasttanh(float x) {
		return RNBO_ApproximateMath::fastertanh(x);
	}
} // RNBO_Gen

using RNBO_Gen::fixnan;
using RNBO_Gen::fixdenorm;
using RNBO_Gen::isdenorm;
//using RNBO_Gen::isnan;
using RNBO_Gen::fract;
using RNBO_Gen::fastsin;
using RNBO_Gen::fastcos;
using RNBO_Gen::fastexp;
using RNBO_Gen::fastpow;
using RNBO_Gen::fasttan;
using RNBO_Gen::fastpow2;
using RNBO_Gen::fastlog2;
using RNBO_Gen::fasttan2;
using RNBO_Gen::fasttanh;

} // RNBO

#endif // #ifndef _RNBO_MATH_H_
