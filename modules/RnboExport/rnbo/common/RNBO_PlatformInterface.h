//
//  _RNBO_PlatformInterface_H_
//
//  Created by Rob Sussman on 5/18/16.
//
//

#ifndef _RNBO_PlatformInterface_H_
#define _RNBO_PlatformInterface_H_

#include "RNBO_Types.h"

#if !defined(RNBO_PLATFORM_ASSERT_WARN)
	#ifdef RNBO_DEBUG
		#define RNBO_PLATFORM_ASSERT_WARN 0
	#else
		#define RNBO_PLATFORM_ASSERT_WARN 1
	#endif
#endif

#if !defined(RNBO_PLATFORM_ERROR_WARN)
	#ifdef RNBO_DEBUG
		#define RNBO_PLATFORM_ERROR_WARN 0
	#else
		#define RNBO_PLATFORM_ERROR_WARN 1
	#endif
#endif


namespace RNBO {

	/**
	 * @brief Standard platform functions for clang/llvm compiled code
	 *
	 * The PlatformInterface minimizes dependencies on standard libraries and
	 * enhances portability.
	 *
	 * Note: It exposes the Engine's environment malloc and free meaning memory
	 * allocated with this interface will stay valid during code compilation
	 * with MCJIT.
	 */
	class PlatformInterface;

	using PlatformInterfacePtr = PlatformInterface*;

	/**
	 * @brief A handler for the PlatformInterface
	 */
	class Platform
	{
	private:

		static PlatformInterfacePtr& instance()
		{
			static PlatformInterface* pInstance = nullptr;
			return pInstance;
		}

	public:

		static PlatformInterface* get()
		{
			return instance();
		}

		static void set(PlatformInterface* platformInterface)
		{
			PlatformInterfacePtr& theInstance = instance();
			theInstance = platformInterface;
		}

	};

	enum class RuntimeError {
		OutOfRange
	};

	class PlatformInterface
	{

	protected:
		PlatformInterface() = default;
		virtual ~PlatformInterface() = default;

	public:
		void resetWarnings() {
#if RNBO_PLATFORM_ERROR_WARN==1
			mErrorWarned = false;
#endif
#if RNBO_PLATFORM_ASSERT_WARN==1
			mAssertWarned = false;
#endif
		}

		/**
		 * @brief Print a message to stdout or some kind of log
		 */
		virtual void printMessage(const char* message) = 0;

		/**
		 * @brief Prints an error message
		 *
		 * Defaults to printMessage.
		 */
		virtual void printErrorMessage(const char* message) {
			printMessage(message);
		}

		/**
		 * @brief Allocates uninitialized storage
		 *
		 * @param bytes the number of bytes of memory to allocate
		 *
		 * @return a pointer to the first byte in the allocated memory block if
		 * allocation succeeds
		 */
		virtual void* malloc(size_t bytes) = 0;

		/**
		 * @brief Allocates memory for an array of objects and initializes all
		 * bytes to zero
		 *
		 * @param num
		 * @param size
		 *
		 * @return a pointer to the first byte in the allocated memory block if
		 * allocation succeeds
		 */
		virtual void* calloc(size_t num, size_t size) = 0;

		/**
		 * @brief Reallocates previously-allocated memory
		 *
		 * @param ptr a pointer to previously-allocated memory
		 * @param bytes the new allocation size
		 *
		 * @return on success, a pointer to the beginning of the newly allocated
		 * memory (and the original pointer is invalidated); on failure, a null
		 * pointer is returned (and the original pointer remains valid)
		 */
		virtual void* realloc(void* ptr, size_t bytes) = 0;

		/**
		 * @brief Deallocates memory
		 *
		 * @param ptr a pointer to memory to deallocate
		 */
		virtual void free(void* ptr) = 0;

		/**
		 * @brief Copy contents of memory from one location to another
		 *
		 * Both memory objects are interpreted as arrays of `unsigned char`.
		 *
		 * @param dest a pointer to the object to copy to
		 * @param src a pointer to the object to copy from
		 * @param n number of bytes to copy
		 *
		 * @return `dest`
		 */
		virtual void* memcpy(void* dest, const void* src, size_t n) = 0;

		/**
		 * @brief Copies memory from one memory location to another
		 *
		 * Both memory objects are interpreted as arrays of `unsigned char`. As
		 * opposed to memcpy, memmove can have overlapping memory regions.
		 *
		 * @param dest a pointer to the object to copy to
		 * @param src a pointer to the object to copy from
		 * @param n number of bytes to copy
		 *
		 * @return copy of `dest`
		 */
		virtual void* memmove(void* dest, const void* src, size_t n) = 0;

		/**
		 * @brief Fill a region of memory with a value
		 *
		 * @param dest a pointer to the object to fill
		 * @param value the fill value
		 * @param n number of bytes to fill
		 *
		 * @return copy of `dest`
		 */
		virtual void* memset(void *dest, int value, size_t n) = 0;

		/**
		 * @brief Get the length of a null-terminated string
		 *
		 * @param s a pointer to the null-terminated string to examine
		 *
		 * @return length of the null-terminated string
		 */
		virtual size_t strlen(const char* s) = 0;

		/**
		 * @brief Compare two null-terminated strings lexicographically
		 *
		 * @param s1 a pointer to a null-terminated string
		 * @param s2 a pointer to a null-terminated string to compare with s1
		 *
		 * @return negative value if s1 is before s2 in lexicographic order,
		 * zero if s1 and s2 are equal, and positive value if s1 is after s2 in
		 * lexicographic order
		 */
		virtual int strcmp(const char* s1, const char* s2) = 0;

		/**
		 * @brief Copy a null-terminated string to a character array
		 *
		 * @param dest a pointer to the character array to write to
		 * @param src a pointer to the null-terminated byte string to copy from
		 * 
		 * @return copy of `dest`
		 */
		virtual char* strcpy(char* dest, const char* src) = 0;

		// string formatting
		virtual void toString(char* str, size_t maxlen, number val) = 0;
		virtual void toString(char* str, size_t maxlen, int val) = 0;
		virtual void toString(char* str, size_t maxlen, unsigned int val) = 0;
		virtual void toString(char* str, size_t maxlen, long val) = 0;
		virtual void toString(char* str, size_t maxlen, long long val) = 0;
		virtual void toString(char* str, size_t maxlen, unsigned long val) = 0;
		virtual void toString(char* str, size_t maxlen, unsigned long long val) = 0;
		virtual void toString(char* str, size_t maxlen, void* val) = 0;

		virtual void abort() = 0;

		/**
		 * @brief Fatal error which should not return
		 */
		virtual void error(RuntimeError e, const char* str) = 0;

		/**
		 * @brief Fatal error if `v` is false
		 */
		virtual void assertTrue(bool v, const char* str) = 0;

		template <typename T>
		T errorOrDefault(RuntimeError e, const char* str, T def) {
#if RNBO_PLATFORM_ERROR_WARN==1
			RNBO_UNUSED(e);
			//warn
			if (once(mErrorWarned)) {
				printErrorMessage(str);
			}
#else
			error(e, str);
#endif
			return def;
		}

		template <typename T>
		T assertTrueOrDefault(bool v, const char* str, T def) {
#if RNBO_PLATFORM_ASSERT_WARN==1
			//warn
			if (!v && once(mAssertWarned)) {
				printErrorMessage(str);
			}
#else
			assertTrue(v, str);
#endif
			return def;
		}
	private:
#if RNBO_PLATFORM_ASSERT_WARN==1 || RNBO_PLATFORM_ERROR_WARN==1
		bool once(bool& v) {
			bool have = v;
			v = true;
			return !have;
		}
#endif
#if RNBO_PLATFORM_ASSERT_WARN==1
		bool mAssertWarned = false;
#endif
#if RNBO_PLATFORM_ERROR_WARN==1
		bool mErrorWarned = false;
#endif
	};

} // namespace RNBO

#endif // #ifndef _RNBO_PlatformInterface_H_
