#ifndef _RNBO_TUPLE_H_
#define _RNBO_TUPLE_H_

#include "RNBO_List.h"

namespace RNBO {

	/**
	 * Fixed-size array. Drop-in replacement for std::array, if you're compiling RNBO without stdlib.
	 */
	template<class T, size_t N> class array {
	public:

		template<typename... Ts> array(Ts ... args)
		{
			// since allocating an array of 0 length is invalid, we always allocate at least length 1
			T values[sizeof...(args) + 1] = {static_cast<T>(args)...};
			for (size_t i = 0; i < sizeof...(args) && i < N; i++) {
				_values[i] = values[i];
			}
		}

		array(const listbase<T>& l) {
			for (size_t i = 0; i < N && i < l.length; i++) {
				_values[i] = l[i];
			}
		}

		// copy constructor
		array(const array<T, N>& a)
		{
			for (size_t i = 0; i < N; i++) {
				_values[i] = a._values[i];
			}
		}

		array<T, N>& operator=(const array<T, N>& a)
		{
			if (&a != this) {
				for (size_t i = 0; i < N; i++) {
					_values[i] = a._values[i];
				}
			}
			return *this;
		}

		constexpr size_t size() const { return N; }
		constexpr bool empty() const { return size() == 0; }

		T& operator[](size_t n) {
			if (n >= N) {
				Platform::get()->errorOrDefault(RuntimeError::OutOfRange, "array index out of range", false /*unused*/);
				_dummy = static_cast<T>(0);
				return _dummy;
			}
			return _values[n];
		}
		const T& operator[](size_t n) const {
			if (n >= N) {
				Platform::get()->errorOrDefault(RuntimeError::OutOfRange, "array index out of range", false /*unused*/);
				return _dummy;
			}
			return _values[n];
		}

		operator listbase<T>() const {
			listbase<T> tmp;
			tmp.reserve(N);
			for (size_t i = 0; i < N; i++) {
				tmp.push(_values[i]);
			}

			return tmp;
		}

	private:
		T		_values[N ? N : 1];
		T _dummy = static_cast<T>(0);
	};

} // namespace RNBO

#endif // #ifndef _RNBO_TUPLE_H_
