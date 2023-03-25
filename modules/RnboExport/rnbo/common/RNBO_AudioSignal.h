#ifndef _RNBO_AUDIOSIGNAL_H_
#define _RNBO_AUDIOSIGNAL_H_

// RNBO_AudioSignal.h -- utilities used by generated code

#include "RNBO_Math.h"
#include "RNBO_Types.h"
#include "RNBO_PlatformInterface.h"

namespace RNBO {

	static SampleIndex msToSamps(MillisecondTime ms, number sr);
	static MillisecondTime sampsToMs(SampleIndex samps, number sr);
	static SampleValue* resizeSignal(SampleValue* sig, size_t oldSize, size_t vs);
	static inline void zeroSignal(SampleValue* sig, size_t size);
	static inline void copySignal(SampleValue *dst, const SampleValue * src, size_t size);
	static inline bool isNaN(number v);
	static void* allocateArray(size_t count, const char* type);
	static number rand01();
	static float bitwiseFloat(unsigned long n);
	static BinOpInt imul(BinOpInt a, BinOpInt b);
	static number pi01();
	static inline int stringCompare(const char *a, const char *b);
	static SampleValue rms(SampleValue *sig, SampleIndex n);
	static SampleValue peakvalue(SampleValue *sig, SampleIndex n);
	static inline int64_t nextpoweroftwo(int64_t v);

	static inline SampleIndex msToSamps(MillisecondTime ms, number sr)
	{
		return SampleIndex(ms * sr * 0.001);
	}

	static inline MillisecondTime sampsToMs(SampleIndex samps, number sr)
	{
		return samps / (sr * 0.001);
	}

	static Sample *resizeSignal(Sample *sig, size_t oldSize, size_t vs)
	{
		// alloc and clear signals

		Sample *newsig = static_cast<Sample*>(Platform::get()->realloc(sig, vs * sizeof(Sample)));
		for (size_t i = oldSize; i < vs; i++) {
			newsig[i] = 0;
		}

		return newsig;
	}

	static inline void zeroSignal(SampleValue* sig, size_t size)
	{
		Platform::get()->memset(sig, 0, size * sizeof(SampleValue));
	}

	static void fillSignal(SampleValue* sig, size_t size, SampleValue value, size_t offset = 0)
	{
		for (size_t i = offset; i < size; i++) {
			sig[i] = value;
		}
	}

	static inline void copySignal(SampleValue *dst, const SampleValue * src, size_t size)
	{
		Platform::get()->memcpy(dst, src, size * sizeof(SampleValue));
	}

	static inline void *allocateArray(size_t count, const char *type)
	{
		RNBO_UNUSED(type);
		return Platform::get()->malloc(count * sizeof(number));
	}

	static inline bool isNaN(number v)
	{
		return RNBO_Math::rnbo_isnan<number>(v) ? true : false;
	}

	static inline bool isFinite(number v)
	{
		return RNBO_Math::rnbo_isfinite<number>(v) ? true : false;
	}

	#define getArrayValueAtIndex(a, index) a[int(index)]

	static inline number rand01()
	{
		return number(rand()) / RAND_MAX;
	}

	static inline float bitwiseFloat(unsigned long n)
	{
		return (*(reinterpret_cast<float*>(&n)));
	}

	static inline BinOpInt imul(BinOpInt a, BinOpInt b)
	{
		return a * b;
	}

	static inline unsigned long systemticks()
	{
		return static_cast<unsigned long>(clock());
	}

	static inline number pi01() // can't call it pi because that is used in some Apple framework that conflicts
	{
		// M_PI is not standard so just returning the value
		return number(3.14159265358979323846);
	}

	static inline int stringCompare(const char *a, const char *b)
	{
		return Platform::get()->strcmp(a, b);
	}

	static inline SampleValue rms(SampleValue *sig, SampleIndex n)
	{
		SampleValue avg = 0;

		for (SampleIndex i = 0; i < n; i++) {
			avg += sig[i] * sig[i];
		}

		return sqrt(avg / n);
	}

	static inline SampleValue peakvalue(SampleValue *sig, SampleIndex n)
	{
		SampleValue peak = 0;
		SampleValue val;

		for (SampleIndex i = 0; i < n; i++) {
			val = fabs(sig[i]);
			if (val > peak)
				peak = val;
		}

		return peak;
	}

	static inline int64_t nextpoweroftwo(int64_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}

	static inline void crash() {
		Platform::get()->abort();
	}

	static inline void crashifNaN(number v) {
		if (isNaN(v) || !isFinite(v)) crash();
	}

	static inline void crashifNaN(SampleValue *sig, Index n) {
		for (Index i = 0; i < n; i++) {
			if (isNaN(sig[i]) || !isFinite(sig[i])) crash();
		}
	}

	template<typename T> void crashifNaN(T v) {
		RNBO_UNUSED(v);
	}

	// we are expecting an array of exactly 4 values !
	using XoshiroState = UInt*;

#ifdef RNBO_USE_FLOAT32
	static const SampleValue EXP2_NEG23 = exp2f(-23.f);

	// splitmix32 suggested by David Blackman and Sebastiano Vigna as a good seed for xoshiro256+:
	/* This is a fixed-increment version of Java 8's SplittableRandom generator
	See http://dx.doi.org/10.1145/2714064.2660195 and
	http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
	It is a very fast generator passing BigCrush, and it can be useful if
	for some reason you absolutely want 64 bits of state. */
	UInt xoshiro_splitmix32(uint64_t seed) {
		uint64_t z = (seed += 0x9e3779b97f4a7c15);
		z = (z ^ (z >> 33)) * 0x62a9d9ed799705f5;
		z = (z ^ (z >> 28)) * 0xcb24d0a5c88c35b3;
		return z >> 32;
	}

	static void xoshiro_reset(SampleValue seed, XoshiroState state) {
		uint64_t x = (uint64_t)(seed * 1E6);
		state[3] = xoshiro_splitmix32(state[2] = xoshiro_splitmix32(state[1] = xoshiro_splitmix32(state[0] = xoshiro_splitmix32(x))));
	}

	// adapted from the xoshiro128+ PRNG, see http://prng.di.unimi.it
	/*  xoshiro128+: Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
	To the extent possible under law, the author has dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.
	See <http://creativecommons.org/publicdomain/zero/1.0/>. */
	inline SampleValue xoshiro_next(XoshiroState state) {
		const UInt result = state[0] + state[3];
		const UInt t = state[1] << 9;
		state[2] ^= state[0];
		state[3] ^= state[1];
		state[1] ^= state[2];
		state[0] ^= state[3];
		state[2] ^= t;
		state[3] = (state[3] << 11) | (state[3] >> 21); // rotl(s[3], 11) => (x << k) | (x >> (32 - k))
		// discard lower 8 bits (exponent), convert to double in 0..2, map to -1..1:
		// exactly quivalent to ldexpf((t_sample)(result >> 8), -23) - 1.0;
		// but much cheaper on ARM CPU
		return ((result >> 8) * EXP2_NEG23) - 1.f;
	}

#else
	static const SampleValue EXP2_NEG52 = exp2(-52);

	// splitmix64 suggested by David Blackman and Sebastiano Vigna as a good seed for xoshiro256+:
	/* This is a fixed-increment version of Java 8's SplittableRandom generator
	See http://dx.doi.org/10.1145/2714064.2660195 and
	http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
	It is a very fast generator passing BigCrush, and it can be useful if
	for some reason you absolutely want 64 bits of state. */
	static UInt xoshiro_splitmix64(UInt seed) {
		UInt z = seed + 0x9e3779b97f4a7c15;
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
		z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
		return z ^ (z >> 31);
	}

	static void xoshiro_reset(SampleValue seed, XoshiroState state) {
		UInt x = (UInt)(seed*1E6);
		state[3] = xoshiro_splitmix64(state[2] = xoshiro_splitmix64(state[1] = xoshiro_splitmix64(state[0] = xoshiro_splitmix64(x))));
	}

	// adapted from the xoshiro256+ PRNG, see http://prng.di.unimi.it
	/*  xoshiro256+: Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
	To the extent possible under law, the author has dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.
	See <http://creativecommons.org/publicdomain/zero/1.0/>. */
	inline SampleValue xoshiro_next(XoshiroState state) {
		const UInt result = state[0] + state[3];
		const UInt t = state[1] << 17;
		state[2] ^= state[0];
		state[3] ^= state[1];
		state[1] ^= state[2];
		state[0] ^= state[3];
		state[2] ^= t;
		state[3] = (state[3] << 45) | (state[3] >> 19); // rotl(s[3], 45) => (x << k) | (x >> (64 - k))
		// discard lower 11 bits, convert to double in 0..2, map to -1..1:
		// equivalent to ldexp((double)(result >> 11), -52) - 1.0;
		return ((result >> 11)*EXP2_NEG52) - 1.0;
	}
#endif	// RNBO_USE_FLOAT32


#define uint32_add(x, y) (UBinOpInt)x + (UBinOpInt)y
#define uint32_trunc(x)	((UBinOpInt)((int64_t)(x)))
#define uint32_rshift(x, y)	((UBinOpInt)x >> (UBinOpInt)y)

} // namespace RNBO

#endif // #ifndef _RNBO_AUDIOSIGNAL_H_
