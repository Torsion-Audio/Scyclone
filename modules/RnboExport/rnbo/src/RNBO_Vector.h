//
//  RNBO_Vector.h
//
//  Created by Jeremy Bernstein on 29.08.16.
//
//

#ifndef RNBO_Vector_h
#define RNBO_Vector_h

#ifdef USE_STD_VECTOR

#include <vector>
#include <algorithm>

namespace RNBO {
	template <class T> using Vector = std::vector<T>;
}

#else // USE_STD_VECTOR

#include <utility> // std::move, doesn't require stdlib
#include <initializer_list>

//#define VECTOR_TESTS
#ifdef VECTOR_TESTS
#include <iostream>
#endif

const int _DEFAULT_VECTOR_SIZE = 4;

namespace RNBO {

	// utility templates

	template<class ForwardIt>
	ptrdiff_t distance(ForwardIt first, ForwardIt last) {
		size_t count = 0;

		for (; first != last; ++first) {
			++count;
		}
		return ptrdiff_t(count);
	}

	template<class InputIt, class Distance>
	void advance(InputIt& it, Distance n) {
		it += n;
	}

	template<class ForwardIt>
	ForwardIt next(ForwardIt it,
				   typename std::iterator_traits<ForwardIt>::difference_type n = 1)
	{
		advance(it, n);
		return it;
	}

	template<class ForwardIt, class T>
	ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T& value) {
		ForwardIt it;
		ptrdiff_t count, step;
		count = RNBO::distance(first,last);

		while (count > 0) {
			it = first;
			step = count / 2;
			RNBO::advance(it, step);
			if (!(value < *it)) {
				first = ++it;
				count -= step + 1;
			} else count = step;
		}
		return first;
	}

	template<class ForwardIt, class T, class Compare>
	ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
		ForwardIt it;
		ptrdiff_t count, step;
		count = RNBO::distance(first, last);

		while (count > 0) {
			it = first;
			step = count / 2;
			RNBO::advance(it, step);
			if (!comp(value, *it)) {
				first = ++it;
				count -= step + 1;
			}
			else {
				count = step;
			}
		}
		return first;
	}

	template<class ForwardIt, class T>
	ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value) {
		ForwardIt it;
		ptrdiff_t count, step;
		count = RNBO::distance(first, last);

		while (count > 0) {
			it = first;
			step = count / 2;
			RNBO::advance(it, step);
			if (*it < value) {
				first = ++it;
				count -= step + 1;
			}
			else
				count = step;
		}
		return first;
	}

	template<class ForwardIt, class T, class Compare>
	ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
		ForwardIt it;
		ptrdiff_t count, step;
		count = RNBO::distance(first,last);

		while (count > 0) {
			it = first;
			step = count / 2;
			RNBO::advance(it, step);
			if (comp(*it, value)) {
				first = ++it;
				count -= step + 1;
			}
			else {
				count = step;
			}
		}
		return first;
	}

	template<class InputIt, class T>
	InputIt find(InputIt first, InputIt last, const T& value) {
		for (; first != last; ++first) {
			if (*first == value) {
				return first;
			}
		}
		return last;
	}

	template< class ForwardIt, class T >
	ForwardIt remove(ForwardIt first, ForwardIt last, const T& value) {
		first = RNBO::find(first, last, value);
		if (first != last)
			for(ForwardIt i = first; ++i != last; )
				if (!(*i == value))
					*first++ = std::move(*i);
		return first;
	}

	template<class InputIt, class UnaryPredicate>
	InputIt find_if(InputIt first, InputIt last, UnaryPredicate p) {
		for (; first != last; ++first) {
			if (p(*first)) {
				return first;
			}
		}
		return last;
	}

	template<class ForwardIt, class UnaryPredicate>
	ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPredicate p) {
		first = RNBO::find_if(first, last, p);
		if (first != last)
			for(ForwardIt i = first; ++i != last; )
				if (!p(*i))
					*first++ = std::move(*i);
		return first;
	}

	/**
	 * @brief A std::vector implementation for use without the C++ Standard Library
	 * 
	 * @tparam T type of element to store in the Vector
	 */
	template <typename T>
	class Vector {

		class auto_array;

	public:

		typedef T* iterator;
		typedef const T* const_iterator;

		Vector()
		: _array(new T[_DEFAULT_VECTOR_SIZE])
		, _capacity(_DEFAULT_VECTOR_SIZE)
		, _size(0)
		{ }

		Vector(int n) // create Vector with n default elements
		: _array(new T[n])
		, _capacity(n)
		, _size(0)
		{ }

		Vector(std::initializer_list<T> l)
		: _array(new T[l.size()])
		, _capacity(l.size())
		, _size(0)
		{
			for (const_iterator it = l.begin(); it != l.end(); it++) {
				_array[_size++] = *it;
			}
		}

		// copy is special
		Vector(const Vector& v)
		: _array(new T[v._capacity])
		, _capacity(v._capacity)
		, _size(0)
		{
			for (const_iterator it = v.begin(); it != v.end(); it++) {
				_array[_size++] = *it;
			}
		}

		// copy-assignment is special
		Vector<T>& operator=(Vector<T>& v) {
			_array = new T[v._capacity];
			_capacity = v._capacity;
			_size = 0;
			for (const_iterator it = v.begin(); it != v.end(); it++) {
				_array[_size++] = *it;
			}
			return *this;
		}

		// move constructor and assignment can be default
		Vector(Vector&& v) {
			_array = v._array;
			_capacity = v._capacity;
			_size = v._size;

			v._array = nullptr;
			v._capacity = 0;
			v._size = 0;
		}

		Vector<T>& operator=(Vector<T>&& v) {
			_array = v._array;
			_capacity = v._capacity;
			_size = v._size;

			v._array = nullptr;
			v._capacity = 0;
			v._size = 0;

			return *this;
		}

		~Vector() {
			delete[] _array;
			// cleanup to avoid trouble if the vector is placed inside another vector
			// we could reimplement the _array as some sort of object so that it's
			// automatically destroyed without requiring a delete[], but this is fine for now.
			_array = nullptr;
			_capacity = 0;
			_size = 0;
		}

		T& operator[](size_t index) {
			return _array[index];
		}

		const T& operator[](size_t index) const {
			return _array[index];
		}

		// T& at(size_t index) {
		// if (index < _size) {
		// 	return _array[index];
		// }
		//  should throw an exception if not
		// }

		void push_back(const T& t) {
			if (_size == _capacity) {
				reserve(_capacity + _DEFAULT_VECTOR_SIZE); // or *2?
			}
			_array[_size] = t;
			_size++;
		}

		void reserve(size_t n) {
			if (n > _capacity) {
				auto_array new_array(new T[n]);
				for (size_t i = 0; i < _size; i++) {
					new_array[i] = std::move(_array[i]);
				}

				delete[] _array;
				_array = new_array.release();
				_capacity = n;
			}
		}

		bool empty() { return begin() == end(); }

		void clear() {
			while (_size) {
				_array[--_size].~T();
			}
		}

		size_t size() { return _size; }
		size_t capacity() { return _capacity; }

		// iterator functions
		iterator begin() const { return _array; }
		iterator end() const { return _array + _size; }

		iterator erase(const_iterator pos) {
			return erase(pos, pos + 1);
		}

		iterator erase(const_iterator first, const_iterator last) {
			if (first == last || first == end()) return iterator(first);

			ptrdiff_t dist = RNBO::distance(first, last);
			rotate(iterator(first), end() - 1, RotateBackward, int(dist));
			resize(_size - dist);
			return iterator(first);
		}

		template <class... Args>
		iterator emplace(const_iterator pos, Args&&... args) {
			ptrdiff_t dist = RNBO::distance(begin(), iterator(pos));

			emplace_back(std::forward<Args>(args)...);
			iterator curr = begin() + dist;
			rotate(curr, end() - 1, RotateForward);
			return curr;
		}

		template <class... Args>
		void emplace_back(Args&&... args) {
			reserve(_size + 1); // ensure space

			iterator curr = end();
			new (static_cast<void*>(curr)) T(std::forward<Args>(args)...); // placement new
			_size++;
		}

		void resize(size_t count) {
			T tmp{};
			resize(count, tmp);
		}

		void resize(size_t count, const T& value) {
			if (_size > count) {
				while (_size > count) {
					_array[--_size].~T();
				}
			} else if (_size < count) {
				reserve(count);
				while (_size < count) {
					new (static_cast<void*>(_array + _size++)) T(std::forward<const T&>(value)); // placement new
				}
			}
		}

		T* data() {
			return _array;
		}

		const T* data() const {
			return _array;
		}

		class reverse_iterator {

		public:

			reverse_iterator(T* p)
			: _pos(p)
			{ }

			reverse_iterator()
			: _pos(0)
			{ }

			T& operator*() { T* tmp = _pos; return *--tmp; }

			T* operator->() { T* tmp = _pos; return --tmp; }

			reverse_iterator& operator++() { _pos--; return *this; }
			reverse_iterator operator++(int) {
				reverse_iterator ret = *this;
				this->operator++();
				return ret;
			}

			reverse_iterator& operator--() { _pos++; return *this; }
			reverse_iterator operator--(int) {
				reverse_iterator ret = *this;
				this->operator--();
				return ret;
			}

			bool operator!=(const reverse_iterator& rhs) { return this->_pos != rhs._pos; }
			
		private:
			
			T*	_pos;
			
		};

		typedef const reverse_iterator const_reverse_iterator;

		reverse_iterator rbegin() {
			return reverse_iterator(end());
		}

		reverse_iterator rend() {
			return reverse_iterator(begin());
		}

	private:

		friend class VectorTests;

		enum RotateDirection {
			RotateForward,
			RotateBackward
		};

		void rotate(iterator first, iterator last, RotateDirection direction, int pos = 1) {
			if (first == last) return;
			if (first == end() || last == end()) return;
			if (pos == 0) return;

			// complicated multi-item rotation as described here:
			// http://www.eis.mdx.ac.uk/staffpages/r_bornat/oldteaching/I2A/slides%209%20circshift.pdf

			ptrdiff_t dist = RNBO::distance(first, last + 1);
			if (pos == dist) return;
			pos = direction == RotateForward ? int(dist - pos) : pos;

			for (int m = 0, count = 0, i = 0, j = 0; count != dist; m++) {
				T tmp = std::move(*first);
				for (i = m, j = m + pos;
					 j != m;
					 i = j, j = int((j + pos < dist) ? (j + pos) : (j + pos - dist)), count++)
				{
					*(first + i) = std::move(*(first + j));
				}
				*(first + i) = std::move(tmp);
				count++;
			}
		}

		T*		_array;
		size_t	_capacity;
		size_t	_size;

		// an auto pointer to use with arrays
		// lets use arrays in create temp and swap idiom
		class auto_array {

		public:

			auto_array(T* t)
			: _ptr(t)
			{ }

			~auto_array() { delete[] _ptr; }

			T* operator->() { return _ptr; }

			T* release() {
				T* tmp(_ptr);
				_ptr = 0;
				return tmp;
			}

			T& operator[](int i) { return _ptr[i]; }
			T& operator[](size_t i) { return _ptr[i]; }

		private:

			T*	_ptr;

		};

	};

}

#ifdef VECTOR_TESTS

namespace RNBO {

	class VectorTests {

	public:

		VectorTests() {
			run();
		}

	private:

		bool testResize() {
			bool haserr = false;
			Vector<int> v = { 1, 2, 3 };
			int output1[] = { 1, 2, 3, 0, 0 };
			int output2[] = { 1, 2 };
			int output3[] = { 1, 2, 6, 6, 6 };

			std::cout << "RNBO::Vector: Testing resize:" << std::endl;

			v.resize(5);
			if (v.size() != sizeof(output1) / sizeof(int)) {
				std::cerr << "RNBO::Vector: wrong size" << std::endl;
				haserr = true;
			}

			for (int* it = v.begin(), i = 0; it != v.end(); it++, i++) {
				if (*it != output1[i]) {
					std::cerr << "RNBO::Vector: bad value at index " << i << " (" << *it << ")" << std::endl;
					haserr = true;
					break;
				}
			}

			v.resize(2);
			if (v.size() != sizeof(output2) / sizeof(int)) {
				std::cerr << "RNBO::Vector: wrong size" << std::endl;
				haserr = true;
			}

			for (int* it = v.begin(), i = 0; it != v.end(); it++, i++) {
				if (*it != output2[i]) {
					std::cerr << "RNBO::Vector: bad value at index " << i << " (" << *it << ")" << std::endl;
					haserr = true;
					break;
				}
			}

			v.resize(5, 6);
			if (v.size() != sizeof(output3) / sizeof(int)) {
				std::cerr << "RNBO::Vector: wrong size" << std::endl;
				haserr = true;
			}

			for (int* it = v.begin(), i = 0; it != v.end(); it++, i++) {
				if (*it != output3[i]) {
					std::cerr << "RNBO::Vector: bad value at index " << i << " (" << *it << ")" << std::endl;
					haserr = true;
					break;
				}
			}

			std::cout << "RNBO::Vector: resize test " << (haserr ? "FAILED" : "PASSED") << std::endl;
			std::cout << std::endl;

			return haserr;
		}

		bool testPushBack() {
			bool haserr = false;
			int output[] = { 7, 5, 16, 8, 25, 13 };
			Vector<int> v = { 7, 5, 16, 8 };

			std::cout << "RNBO::Vector: Testing push_back:" << std::endl;

			// Add two more integers to vector
			v.push_back(25);
			v.push_back(13);

			if (v.size() != sizeof(output) / sizeof(int)) {
				std::cerr << "RNBO::Vector: wrong size" << std::endl;
				haserr = true;
			}

			for (int* it = v.begin(), i = 0; it != v.end(); it++, i++) {
				if (*it != output[i]) {
					std::cerr << "RNBO::Vector: bad value at index " << i << " (" << *it << ")" << std::endl;
					haserr = true;
					break;
				}
			}

			std::cout << "RNBO::Vector: push_back test " << (haserr ? "FAILED" : "PASSED") << std::endl;
			std::cout << std::endl;

			return haserr;
		}

		bool testEmplace() {
			bool haserr = false;
			int output[] = { 5, 10, 2, 4, 15, 6, 20 };
			Vector<int> v = { 2, 4, 6 };

			std::cout << "RNBO::Vector: Testing emplace and emplace_back:" << std::endl;

			// Add two more integers to vector
			int* result = v.emplace(v.begin(), 10); // this also tests a resize
			v.emplace(result, 5); // test return value
			v.emplace(v.begin() + 4, 15);
			v.emplace_back(20);

			if (v.size() != sizeof(output) / sizeof(int)) {
				std::cerr << "RNBO::Vector: wrong size" << std::endl;
				haserr = true;
			}

			for (int* it = v.begin(), i = 0; it != v.end(); it++, i++) {
				if (*it != output[i]) {
					std::cerr << "RNBO::Vector: bad value at index " << i << " (" << *it << ")" << std::endl;
					haserr = true;
					break;
				}
			}

			std::cout << "RNBO::Vector: emplace test " << (haserr ? "FAILED" : "PASSED") << std::endl;
			std::cout << std::endl;

			return haserr;
		}

		bool testErase() {
			bool haserr = false;
			int output[] = { 5, 2, 4, 20 };
			Vector<int> v = { 8, 5, 10, 2, 4, 15, 6, 20, 22 };

			std::cout << "RNBO::Vector: Testing erase:" << std::endl;

			int* rv;
			rv = v.erase(v.begin() + 2); // remove single element
			if (rv != v.begin() + 2) {
				std::cerr << "RNBO::Vector: bad return value (1)" << std::endl;
				haserr = true;
			}
			rv = v.erase(v.begin() + 4, v.begin() + 6); // remove two elements
			if (rv != v.begin() + 4) {
				std::cerr << "RNBO::Vector: bad return value (2)" << std::endl;
				haserr = true;
			}
			rv = v.erase(v.end() - 1); // erase last element
			if (rv != v.end()) {
				std::cerr << "RNBO::Vector: bad return value (3)" << std::endl;
				haserr = true;
			}
			rv = v.erase(v.begin()); // erase first element
			if (rv != v.begin()) {
				std::cerr << "RNBO::Vector: bad return value (4)" << std::endl;
				haserr = true;
			}

			if (v.size() != sizeof(output) / sizeof(int)) {
				std::cerr << "RNBO::Vector: wrong size" << std::endl;
				haserr = true;
			}

			for (int* it = v.begin(), i = 0; it != v.end(); it++, i++) {
				if (*it != output[i]) {
					std::cerr << "RNBO::Vector: bad value at index " << i << " (" << *it << ")" << std::endl;
					haserr = true;
					break;
				}
			}

			std::cout << "RNBO::Vector: erase test " << (haserr ? "FAILED" : "PASSED") << std::endl;
			std::cout << std::endl;

			return haserr;
		}

		bool testIterators() {
			bool haserr = false;
			Vector<int> v;

			std::cout << "RNBO::Vector: Testing iterators:" << std::endl;

			// insert elements from 1 to 9
			for (int i=1; i<=9; ++i) {
				v.push_back(i);
			}

			// find position of element with value 5
			Vector<int>::iterator pos;
			pos = find(v.begin(), v.end(), 5);

			// print value to which iterator pos refers
			// std::cout << "pos:  " << *pos << std::endl;

			// convert iterator to reverse iterator rpos
			Vector<int>::reverse_iterator rpos(pos);

			// print value to which reverse iterator rpos refers
			// std::cout << "rpos: " << *rpos << std::endl;

			haserr = !(*pos == 5 && *rpos == 4);

			std::cout << "RNBO::Vector: iterator test " << (haserr ? "FAILED" : "PASSED") << std::endl;
			std::cout << std::endl;

			return haserr;
		}

		bool testMisc() {
			bool haserr = false;
			Vector<int> v;

			std::cout << "RNBO::Vector: Testing misc:" << std::endl;

			v.reserve(200);
			if (v.size() != 0) {
				std::cerr << "RNBO::Vector: bad size (should be 0, is " << v.size() << std::endl;
				haserr = true;
			}
			if (v.capacity() != 200) {
				std::cerr << "RNBO::Vector: bad capacity (should be 200, is " << v.capacity() << std::endl;
				haserr = true;
			}

			v.erase(v.begin()); // should do nothing, but might crash! :-)

			size_t n = 50;
			while (n--) {
				v.push_back(n);
			}
			if (v.size() != 50) {
				std::cerr << "RNBO::Vector: bad size (should be 50, is " << v.size() << std::endl;
				haserr = true;
			}
			if (v.capacity() != 200) {
				std::cerr << "RNBO::Vector: bad capacity (should be 200, is " << v.capacity() << std::endl;
				haserr = true;
			}

			v.clear();
			if (v.size() != 0) {
				std::cerr << "RNBO::Vector: bad size (should be 0, is " << v.size() << std::endl;
				haserr = true;
			}
			if (v.capacity() != 200) {
				std::cerr << "RNBO::Vector: bad capacity (should be 200, is " << v.capacity() << std::endl;
				haserr = true;
			}

			int* data = v.data();
			if (data != v._array) {
				std::cerr << "RNBO::Vector: invalid data" << std::endl;
				haserr = true;
			}

			Vector<int> v2{ 3, 1, 4 };

			auto it = v2.begin();
			auto nx = std::next(it, 2);

			if (*it != 3 || *nx != 4) {
				std::cerr << "RNBO::Vector: std::next is broken" << std::endl;
				haserr = true;
			}

			Vector<int> v3 = { 1, 1, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6 };
			int output[] = { 4, 4, 4 };

			auto lower = RNBO::lower_bound(v3.begin(), v3.end(), 4);
			auto upper = RNBO::upper_bound(v3.begin(), v3.end(), 4);

			for (int* it = lower, i = 0; it != upper; it++, i++) {
				if (*it != output[i]) {
					std::cerr << "RNBO::Vector: lower_bound/upper_bound are broken" << std::endl;
					haserr = true;
					break;
				}
			}

			std::cout << "RNBO::Vector: misc test " << (haserr ? "FAILED" : "PASSED") << std::endl;
			std::cout << std::endl;

			return haserr;
		}

		void run() {
			static bool ran = false;

			if (ran) return;

			testPushBack();
			testEmplace();
			testErase();
			testResize();
			testIterators();
			testMisc();

			ran = true;

		}
	};

}

#endif // VECTOR_TESTS

#endif // USE_STD_VECTOR

#endif // RNBO_Vector_h
