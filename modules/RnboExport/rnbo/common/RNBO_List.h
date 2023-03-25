#ifndef _RNBO_LIST_H_
#define _RNBO_LIST_H_

#include "RNBO_Types.h"
#include "RNBO_PlatformInterface.h"

#ifdef RNBO_NOSTDLIB
#include "RNBO_UniquePtr.h"
#else
#include <memory>
#endif // RNBO_NOSTDLIB

namespace RNBO {

	static const size_t listChunkSize = 8;

	/**
	 * @brief A container similar to a JavaScript array
	 *
	 * Elements of listbase are stored contiguously in memory making this
	 * container more similar to std::vector than std::list. The listbase is
	 * resized only when adding elements; memory is allocated when elements are
	 * added, but memory is not freed when removing elements.
	 *
	 * @tparam type of object stored in the container
	 */
	template<typename T> class listbase {
	public:

		/**
		 * @brief Default constructor
		 */
		listbase()
		: length(0)
		, _values(nullptr)
		, _allocatedLength(0)
		{
			allocate(length, false);
		}

		/**
		 * @brief Constructor
		 *
		 * @tparam type of items in the listbase
		 *
		 * @code{.cpp}
		 * listbase<number> foo(3.14, 5.2, 4);  // foo = [3.14, 5.2, 4.0]
		 * listbase<int> bar(74, 42, 99, 44100); // bar = [74, 42, 99, 44100]
		 * @endcode
		 */
		template<typename... Ts> listbase(Ts ... args)
		: length(sizeof...(args))
		, _values(nullptr)
		, _allocatedLength(0)
		{
			allocate(length, false);
			T listValues[sizeof...(args)] = {static_cast<T>(args)...};
			for (size_t i = 0; i < length; i++) {
				_values[i] = listValues[i];
			}
		}

		/**
		 * @brief Copy constructor
		 *
		 * @tparam listbase the listbase to copy from
		 *
		 * @code{.cpp}
		 * listbase<int> foo(3.14, 5.2, 4);
		 * listbase<int> bar(foo);
		 * @endcode
		 */
		listbase(const listbase &l)
		: length(l.length)
		, _values(nullptr)
		, _allocatedLength(0)
		{
			allocate(length, false);
			for (size_t i = 0; i < length; i++) {
				_values[i] = l._values[i];
			}
		}

		/**
		 * @brief Move constructor
		 */
		listbase(listbase&& l)
		: length(l.length)
		, _values(l._values)
		, _allocatedLength(l._allocatedLength)
		{
			l._allocatedLength = 0;
			l._values = nullptr;
		}

		/**
		 * @brief Destructor
		 */
		~listbase()
		{
			if (_values) {
				Platform::get()->free(_values);
				_values = nullptr;
			}
		}

		/**
		 * @brief Get a reference to the element at specified location with
		 * bounds checking
		 *
		 * @code{.cpp}
		 * listbase<int> foo(3, 1, 4);
		 * foo[2] = 74; // foo = [3, 1, 74]
		 * foo[3] = 100; // returns runtime error and doesn't assign 100 to
		 *               // unowned memory
		 * @endcode
		 */
		T& operator[](size_t i) {
			if (i >= length) {
				Platform::get()->errorOrDefault(RuntimeError::OutOfRange, "list index out of range", false /*unused*/);
				_dummy = static_cast<T>(0);
				return _dummy;
			}
			return _values[i];
		}

		/**
		 * @brief Get an element at a specified location with bounds checking
		 *
		 * @code{.cpp}
		 * listbase<int> foo(3, 1, 4);
		 * int x = foo[2]; // x = 4
		 * int y = foo[3]; // returns runtime error and sets y = 0
		 * @endcode
		 */
		T operator[](size_t i) const {
			if (i >= length) {
				return Platform::get()->errorOrDefault(RuntimeError::OutOfRange, "list index out of range", 0);
			}
			return _values[i];
		}

		listbase* operator->() {
			return this;
		}

		const listbase* operator->() const {
			return this;
		}

		/**
		 * @brief Copy assignment operator
		 */
		listbase& operator=(const listbase& l)
		{
			if (&l != this) {
				length = l.length;
				allocate(length, false);
				for (size_t i = 0; i < length; i++) {
					_values[i] = l._values[i];
				}
			}
			return *this;
		}

		/**
		 * @brief Append an item to the list
		 *
		 * @param item an item of the same type as other elements in the
		 * list
		 *
		 * @code{.cpp}
		 * listbase<int> foo();
		 * foo.push(2); // foo = [2];
		 * @endcode
		 */
		void push(T item) {
			allocate(length + 1, true);
			_values[length] = item;
			length++;
		}

		/**
		 * @brief Remove an item from the end of the list and return it
		 *
		 * @return the last item from the list
		 *
		 * @code{.cpp}
		 * listbase<number> foo(3.1, 4.2, 7.4);
		 * number x = foo.pop(); // x = 7.4
		 * @endcode
		 */
		T pop() {
			T tmp = 0;
			if (length > 0) {
				tmp = _values[length - 1];
				length--;
			}
			return tmp;
		}

		/**
		 * @brief Remove the first item from the list and return it
		 *
		 * Note that this moves all items in memory from locations `1..n` to
		 * `0..n-1`.
		 *
		 * @return the first item of the list if one exists, 0 otherwise
		 */
		T shift() {
			if (length == 0) {
				return Platform::get()->errorOrDefault(RuntimeError::OutOfRange, "cannot shift out of empty list", 0);
			}
			T tmp = _values[0];
			Platform::get()->memmove(_values, _values + 1, (length - 1) * sizeof(T));
			length--;
			return tmp;
		}

		/**
		 * @brief Create a new listbase by copying the original and appending an item
		 *
		 * @param item the item to append to the list copy
		 * @return a copy of the original list with `item` appended
		 *
		 * @code{.cpp}
		 * listbase<number> foo(3.1, 4.2, 7.4);
         * listbase<number> bar = foo.concat(0.5); // foo = [3.1, 4.2, 7.4]
		 *                                         // bar = [3.1, 4.2, 7.4, 0.5]
		 * @endcode
		 */
		listbase concat(T item) const {
			listbase tmp(*this);
			tmp.push(item);
			return tmp;
		}

		/**
		 * @brief Concatenate a list with another
		 *
		 * @param item the item to append to the list copy
		 * @return a copy of the original list with `item` appended
		 *
		 * @code{.cpp}
		 * listbase<number> foo(3.1, 4.2);
		 * listbase<number> bar(5.0, 1.0);
		 * listbase<number> meow = foo.concat(bar);  // meow = [3.1, 4.2, 5.0, 1.0]
		 * @endcode
		 */
		listbase concat(const listbase& item) const {
			listbase tmp(*this);
			tmp.allocate(tmp.length + item.length, true);
			for (size_t i = 0; i < item.length; i++) {
				tmp._values[tmp.length] = item[i];
				tmp.length++;
			}

			return tmp;
		}

		/**
		 * @brief Fill a list with a value
		 * 
		 * @param value the value to fill with
		 * @param start the index to begin filling from
		 * @param end the index to end filling at; `end = 0` sets the endpoint
		 * to the last element
		 * @return a reference to the list
		 */
		listbase& fill(T value, size_t start = 0, size_t end = 0)
		{
			if (end == 0) end = length;
			allocate(end, true);
			if (end > length) length = end;
			for (size_t i = start; i < end; i++) {
				_values[i] = value;
			}

			return *this;
		}

		/**
		 * @brief Add one or more elements to the beginning of the list
		 * 
		 * @tparam Ts type of elements to add to the list
		 * @param args elements to add
		 */
		template<typename... Ts> void unshift(Ts ... args) {
			splice(0, 0, args...);
		}

		/**
		 * @brief Modify part a list in place
		 * 
		 * @tparam Ts 
		 * @param start the beginning index. If `start` is greater than the
		 * length of the list, splice uses the length of the list.
		 * @param deleteCount the number of elements to remove from the
		 * beginning of the list
		 * @param args items to add to the list
		 */
		template<typename... Ts> listbase<T> splice(Int start, size_t deleteCount, Ts ... args)
		{
			if (start < 0) start += length;
			if (start < 0) start = 0;
			size_t iStart = (size_t)start;
			if (iStart >= length) {
				deleteCount = 0;
				iStart = length;
			}

			if (iStart + deleteCount > length) deleteCount = length - iStart;

			const size_t addLength = sizeof...(args);
			const long diff = (long)(addLength - deleteCount);

			listbase<T> deletedItems;
			deletedItems.allocate(deleteCount, false);
			for (size_t i = 0; i < deleteCount; i++) {
				deletedItems.push(_values[iStart + i]);
			}

			long newLength = (long)(length) + diff;
			if (newLength <= 0) {
				length = 0;
				return deletedItems;
			}

			allocate(static_cast<size_t>(newLength), true);

			if (diff < 0) {
				for (long i = (long)start - diff; i < (long)length; i++) {
					_values[i + diff] = _values[i];
				}
			} else if (diff > 0) {
				for (long i = (long)(length - 1); i >= (long)start; i--) {
					_values[i + diff] = _values[i];
				}
			}

			// since allocating an array of 0 length is invalid, we always allocate at least length 1
			T addValues[(sizeof...(args)) + 1] = {static_cast<T>(args)...};
			for (size_t i = 0; i < addLength; i++) {
				_values[i + start] = addValues[i];
			}

			length = static_cast<size_t>(newLength);

			return deletedItems;
		}

		/**
		 * @brief Get a shallow copy of a portion of a list
		 * 
		 * @param start the beginning index
		 * @param end the end index; 0 is the final element and negative values
		 * are offset from the end
		 * @return a shallow copy of a portion of a list
		 */
		listbase slice(int start = 0, int end = 0) const {
			listbase tmp;
			int ilen = static_cast<int>(this->length);

			//negative is offset from end
			if (start < 0) {
				start = ilen + start;
				if (start < 0) {
					start = 0;
				}
			}

			//offset from end
			if (end <= 0) {
				end = ilen + end;
			}

			//bounds check
			if (start >= ilen || ilen == 0) {
				return tmp;
			}
			if (end > ilen) {
				end = ilen;
			}

			//allocate and copy
			tmp.allocate(static_cast<size_t>(end - start), false);
			for (int i = start; i < end; i++) {
				tmp._values[tmp.length] = _values[i];
				tmp.length++;
			}

			return tmp;
		}

		/**
		 * @brief Determine whether an element is in the list
		 * 
		 * @param value the element to search for
		 * @param fromIndex the position to begin searching from
		 * @return true if the value is found, false otherwise
		 */
		bool includes(T value, int fromIndex = 0) const {
			if (fromIndex < 0) {
				fromIndex = int(length) + fromIndex;
				if (fromIndex < 0) fromIndex = 0;
			}

			for (size_t i = size_t(fromIndex); i < length; i++) {
				if (_values[i] == value) return true;
			}

			return false;
		}

		/**
		 * @brief Find the first index of an element's occurrence in the list
		 * 
		 * @param value the element to locate
		 * @param fromIndex the index to begin searching from. If the value is
		 * negative, begins searching from 0.
		 * @return the index of the first matching element in the list; -1 if
		 * the element is not found
		 */
		int indexOf(T value, int fromIndex = 0) const {
			if (fromIndex < 0) {
				fromIndex = int(length) + fromIndex;
				if (fromIndex < 0) fromIndex = 0;
			}

			for (size_t i = size_t(fromIndex); i < length; i++) {
				if (_values[i] == value) return (int)i;
			}

			return -1;
		}

		/**
		 * @brief Reverse the list in place
		 * 
		 * @return a reference to the list 
		 */
		listbase& reverse() {
			size_t len2 = length >> 1;

			for (size_t i = 0; i < len2; ++i) {
				T tmp = _values[i];
				size_t target = length - i - 1;
				_values[i] = _values[target];
				_values[target] = tmp;
			}

			return *this;
		}

		/**
		 * @brief Pre-allocate memory for a list
		 * 
		 * @param size the number of elements for which to reserve space
		 */
		void reserve(size_t size) {
			allocate(size, true);
		}

		/**
		 * @brief the number of elements in the list
		 */
		size_t	length = 0;

		/**
		 * @brief get a pointer to the internal values, for memcopy etc
		 */
		T* inner() const { return _values; }

	protected:
		void allocate(size_t size, bool keepValues)
		{
			if (size > _allocatedLength) {
				T *old_values = _values;

				_allocatedLength = listChunkSize + (size_t(size/listChunkSize)) * listChunkSize;
				_values = (T*) Platform::get()->malloc(sizeof(T) * _allocatedLength);

				if (keepValues) {
					Platform::get()->memcpy(_values, old_values, length * sizeof(T));
				}

				if (old_values) {
					Platform::get()->free(old_values);
				}
			}
		}

		/**
		 * @brief the contents of the list
		 */
		T*	_values;

		/**
		 * @brief how much memory has been allocated for the list
		 */
		size_t	_allocatedLength;

		/**
		 * @brief a dummy variable to accept out of bounds array assignments
		 * rather than allow out of bounds memory acess
		 */
		T _dummy = static_cast<T>(0);
	};

	using list = listbase<number>;
	using indexlist = listbase<Index>;
	using intlist = listbase<int>;


	/**
	 * @brief A simple container class to make it possible to have lists and
	 * numbers in the same array
	 *
	 * Mainly needed internally because we do not want lists to implicitly
	 * convert to numbers which could hide code generation errors
	 */
	class ListNum {
	public:
		ListNum()
		: _val()
		{}

		ListNum(number val)
		: _val(val)
		{}

		ListNum(list val)
		: _val(val)
		{}

		operator number() const { return _val.length > 0 ? _val[0] : 0; }
		operator list() const { return _val; }

		list& operator->() {
			return _val;
		}

		const list& operator->() const {
			return _val;
		}

	private:
		list _val;
	};

#ifdef RNBO_NOSTDLIB
	using UniqueListPtr = UniquePtr<list>;
#else
	using UniqueListPtr = std::unique_ptr<listbase<number>>;
#endif // RNBO_NOSTDLIB

} // namespace RNBO

#endif // #ifndef _RNBO_LIST_H_
