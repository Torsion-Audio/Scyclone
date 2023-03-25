//
//  _RNBO_Std_H_
//
//  Created by Jeremy Bernstein on 7/12/16.
//
//

#ifndef _RNBO_Std_H_
#define _RNBO_Std_H_

#include "RNBO_CompilerMacros.h"

#define RNBO_UNUSED(_var_) ((void)_var_);

#if defined(__clang__)
	#define RNBO_PUSH_DISABLE_PEDANTIC_WARNINGS \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Wpedantic\"")
#elif defined (__GNUC__)
	#define RNBO_PUSH_DISABLE_PEDANTIC_WARNINGS \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wpedantic\"")
#else
	#define RNBO_PUSH_DISABLE_PEDANTIC_WARNINGS
#endif

#if defined(__clang__)
	#define RNBO_POP_DISABLE_PEDANTIC_WARNINGS \
	_Pragma("clang diagnostic pop")
#elif defined (__GNUC__)
	#define RNBO_POP_DISABLE_PEDANTIC_WARNINGS \
	_Pragma("GCC diagnostic pop")
#else
	#define RNBO_POP_DISABLE_PEDANTIC_WARNINGS
#endif

#if defined(__clang__)
	#define RNBO_PUSH_DISABLE_WARNINGS \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Weverything\"")
#elif defined (__GNUC__)
	#define RNBO_PUSH_DISABLE_WARNINGS \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Weverything\"")
#else
	#define RNBO_PUSH_DISABLE_WARNINGS
#endif

#if defined(__clang__)
	#define RNBO_POP_DISABLE_WARNINGS \
	_Pragma("clang diagnostic pop")
#elif defined (__GNUC__)
	#define RNBO_POP_DISABLE_WARNINGS \
	_Pragma("GCC diagnostic pop")
#else
	#define RNBO_POP_DISABLE_WARNINGS
#endif

#ifndef RNBO_NOSTDLIB

#define USE_STD_VECTOR

#include <stddef.h>
#include <stdint.h>

// RNBO_AudioSignal.h
#ifndef clock
#include <time.h>
#endif

#ifndef rand
#include <stdlib.h>
#endif

#else // RNBO_NOSTDLIB

// size_t
#ifdef __APPLE__
using size_t = unsigned long; // 32- and 64-bit definition
using intptr_t = long;
#else // !__APPLE__
#if defined(__LP64__)
using size_t = unsigned long;
using intptr_t = long;
#elif defined(_WIN64)
using size_t = unsigned long long;
using intptr_t = long long;
#else // !__LP64__ && !_WIN64, must be 32 bit
using size_t = unsigned int;
using intptr_t = int;
#endif // __LP64__
#endif // __APPLE__


#ifndef SIZE_MAX
#if defined(__x86_64__) || defined(_IA64) || defined(__IA64__) || defined(_M_X64) || defined(__aarch64__)
//64 bits
#define SIZE_MAX 0xFFFFFFFFFFFFFFFF
#else
//32 bits
#define SIZE_MAX 0xFFFFFFFF
#endif
#endif

using uint8_t = unsigned char;
using int8_t = char;
using uint16_t = unsigned short;
using int16_t = short;

using int64_t = long long;
using uint64_t = unsigned long long;
#ifdef _WIN32
using int32_t = int;
#else
using int32_t = __INT32_TYPE__;
#endif

#ifndef ARM_CORTEX
using uint32_t = unsigned int;
#else

#ifdef _WIN32
using uint32_t = unsigned int;
#else
using uint32_t = __UINT32_TYPE__;
#endif

#endif  // RNBO_NOSTDLIB

extern "C" {
	void	*malloc(size_t);
	void	*calloc(size_t num, size_t size);
	void	 free(void *);
	void	*realloc(void *, size_t);
	static	void *__dso_handle = nullptr; // necessary on Debian-based (Ubuntu, Xenomai) systems
}

extern "C" {
	// RNBO_AudioSignal.h
	unsigned long	clock(void);
	int				rand(void);

#ifdef RAND_MAX
#undef RAND_MAX
#endif

// TODO: the following is a temporary hack, see #12824.
#ifdef _WIN32
	const int		RAND_MAX = 0x7fff;
#else
	const int		RAND_MAX = 0x7fffffff;
#endif
	const int		CLOCKS_PER_SEC = 1000000;

	// math functions defined in RNBO_Math.h (with declarations in RNBO_CMath.h)
}

// implementations required to build without libc++
// see RNBO_PlatfomInterfaceNoStdLib.cpp

void* operator new (size_t size) throw();
void *operator new[](size_t size) throw();
void operator delete (void *p) throw();
void operator delete[](void *p) throw();
extern "C" void __cxa_pure_virtual();

#endif // #ifdef RNBO_NOSTDLIB

#endif // #ifndef _RNBO_Std_H_

