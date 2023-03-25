#pragma once

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#if !defined(GNUC_PREREQ)
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define GNUC_PREREQ(maj, min, patch)                                  \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >=          \
   ((maj) << 20) + ((min) << 10) + (patch))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define GNUC_PREREQ(maj, min, patch)                                  \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
#else
#define GNUC_PREREQ(maj, min, patch) 0
#endif
#endif

#if defined(__clang__)
# if defined(__APPLE__)
#  define APPLE_CLANG_VERSION __clang_major__
#  define CLANG_VERSION 0
# else
#  define APPLE_CLANG_VERSION 0
#  define CLANG_VERSION __clang_major__
# endif
#else
# define APPLE_CLANG_VERSION 0
# define CLANG_VERSION 0
#endif

// NB: clang/llvm on Windows will define _MSC_VER so be careful
#if defined(_MSC_VER)
#define MSVC_VERSION _MSC_VER
#else
#define MSVC_VERSION 0
#endif

#if __has_attribute(noinline) || GNUC_PREREQ(3, 4, 0)
#define ATTRIBUTE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define ATTRIBUTE_NOINLINE
#endif

#if __has_attribute(always_inline) || GNUC_PREREQ(4, 0, 0)
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define ATTRIBUTE_ALWAYS_INLINE __forceinline
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

#if __has_attribute(unused) || GNUC_PREREQ(4, 0, 0)
#define ATTRIBUTE_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
#define ATTRIBUTE_UNUSED // to suppress warning, wrap code with #pragma warning(disable/default:4505)
#else
#define ATTRIBUTE_UNUSED
#endif

#undef CLANG_VERSION
#undef MSVC_VERSION
#undef GNUC_PREREQ
