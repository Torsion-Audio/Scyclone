//
//  RNBO_String.h
//
//  Created by Jeremy Bernstein on 5/9/16.
//
//

#ifndef _RNBO_String_H_
#define _RNBO_String_H_

#include "RNBO_PlatformInterface.h"

namespace RNBO {

	/**
	 * @brief A string implementation without C++ standard library dependencies
	 */
	class String {

	public:

		/**
		 * @brief Construct a new String object
		 */
		String() : _ptr(new char[1]), _len(0), _truelen(1) { _ptr[0] = '\0'; }

		/**
		 * @brief Copy-construct a new String object
		 *
		 * @param str a null-terminated string
		 */
		String(const char* str) : _ptr(nullptr), _len(0), _truelen(0) { copy(str); }

		/**
		 * @brief Copy-construct a new String object
		 *
		 * @param str another String
		 */
		String(String const& str) : _ptr(nullptr), _len(0), _truelen(0) { copy(str._ptr); }

		/**
		 * @brief Destroy the String object
		 */
		~String() { delete[] _ptr; }

		/**
		 * @brief Get the length of the String
		 *
		 * @return the String length
		 */
		size_t len() const { return _len; }

		/**
		 * @brief Clear the contents of the String
		 */
		void clear() { if (_ptr) _ptr[0] = '\0'; _len = 0; }

		/**
		 * @brief Get whether the String is empty
		 *
		 * @return true if the String has length 0
		 * @return false otherwise
		 */
		bool empty() const { return _len == 0; }

		/**
		 * @brief Append additional characters to the end of the String
		 *
		 * @param astr a null-terminated string to append to this String
		 */
		void append(const char* astr) {
			size_t alen = Platform::get()->strlen(astr);
			size_t newlen = alen + _len;

			if (newlen >= _truelen) {
				char* oldptr = _ptr;

				if (_truelen < 16) _truelen = 16;
				while (newlen >= _truelen)
					_truelen *= 2;
				_ptr = new char[_truelen];
				Platform::get()->memcpy(_ptr, oldptr, _len);
				if (oldptr)
					delete[] oldptr;
			}
			Platform::get()->memcpy(_ptr + _len, astr, alen);
			_ptr[newlen] = '\0';
			_len = newlen;
		}

		/**
		 * @brief Append a String to this String
		 *
		 * @param astr a String to append
		 */
		void append(String& astr) {
			append(astr._ptr);
		}

		/**
		 * @brief Get the String's underlying null-terminated string
		 *
		 * @return the null-terminated string
		 */
		const char* c_str() const { return const_cast<const char*>(_ptr); }

		/**
		 * @brief Copy-assignment operator
		 *
		 * @param origin String to copy
		 * @return String&
		 */
		String& operator = (const String& origin) { copy(origin._ptr); return *this; }

		/**
		 * @brief Copy-assignment operator
		 *
		 * @param origin null-terminated string to copy
		 * @return String&
		 */
		String& operator = (const char* origin) { copy(origin); return *this; }

		/**
		 * @brief Addition assignment operator
		 *
		 * Append a String to this String
		 *
		 * @param origin the other String to append to this String
		 * @return String&
		 */
		String& operator += (const String& origin) { append(origin._ptr); return *this; }

		/**
		 * @brief Addition assignment operator
		 *
		 * Append a null-terminated to this String
		 *
		 * @param origin the null-terminated string to append to this String
		 * @return String&
		 */
		String& operator += (const char* origin) { append(origin); return *this; }

		/**
		* @brief Concatenate two String objects
		*
		* @param str1 first String
		* @param str2 second String
		* @return a new String which is the concatenation of the inputs
		*/
		friend String operator+ (String& str1, String& str2);

		/**
		 * @brief Compare String objects for equality
		 *
		 * @param other the String to compare
		 * @return true if the contents of this String and `other` are equal
		 * @return false otherwise
		 */
		bool operator==(const String& other) const {
			return Platform::get()->strcmp(_ptr, other._ptr) == 0;
		}

		/**
		 * @brief Compare if this String is lexicographically smaller than another
		 *
		 * @param other the String to compare with this String
		 * @return true if this String is lexicographically smaller than the argument
		 * @return false otherwise
		 */
		bool operator<(const String& other) const {
			return (Platform::get()->strcmp(_ptr, other._ptr)) < 0 ? true : false;
		}

		/**
		 * @brief Compare if this String is lexicographically larger than another
		 *
		 * @param other the String to compare with this String
		 * @return true if this String is lexicographically larger than the argument
		 * @return false otherwise
		 */
		bool operator>(const String& other) const {
			return (Platform::get()->strcmp(_ptr, other._ptr)) > 0 ? true : false;
		}

		friend struct StringHasher;

	private:

		int copy(const char* origin) {
			clear();
			append(origin);
			return 0;
		}

		char*  _ptr;
		size_t _len;
		size_t _truelen;
	};

	/**
	 * @brief Concatenate two String objects
	 *
	 * @param str1 first String
	 * @param str2 second String
	 * @return a new String which is the concatenation of the inputs
	 */
	inline String operator+ (String& str1, String& str2) {
		String* newstr = new String(str1);
		newstr->append(str2._ptr);
		return *newstr;
	}

	/**
	 * @brief A function object to use when generating hashes of String objects
	 */
	struct StringHasher {

		/**
		 * @brief Calculate the hash of the argument
		 *
		 * @param k the String to hash
		 * @return size_t the hash value
		 */
		size_t operator()(const String& k) const {
			const char* str = k.c_str();
			return TAG(str);
		}
	};

} // namespace RNBO

#endif // #ifndef _RNBO_String_H_
