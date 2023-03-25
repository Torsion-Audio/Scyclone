//
//   RNBO_UniquePtr.h
//   Created: 30 Jun 2016 6:66:66pm
//   Author:  Jeremy Bernstein
//
//

#ifndef _RNBO_UNIQUEPTR_H_
#define _RNBO_UNIQUEPTR_H_

namespace RNBO {

	/**
	 * @brief A destruction policy for a smart pointer like std::default_delete
	 *
	 * Used as the default delete method for a UniquePtr.
	 */
	template <class T>
	struct default_delete {

		/**
		 * @brief Calls delete on a pointer
		 *
		 * @param p an object or array to delete
		 */
		void operator() (T* p) const
		{
			delete p;
		}
	};

	/**
	 * @brief An smart pointer like std::unique_ptr
	 *
	 * A unique pointer is a smart pointer that owns and manages another object
	 * through a pointer and disposes of that object when the UniquePtr goes
	 * out of scope. Ownership is enforced by deletion of copy construction and
	 * copy assignment.
	 *
	 * @tparam T the type of object to point to
	 * @tparam Deleter a callable delete for object destruction
	 * @see default_delete
	 *
	 * @code{.cpp}
	 * {
	 *     auto myPtr = UniquePtr<int>(new int(74));
	 * } // myPtr is out of scope and the memory allocated is now freed
	 * @endcode{}
	 */
	template <class T, class Deleter = default_delete<T> >
	class UniquePtr {

	public:

		/**
		 * @brief Construct a UniquePtr
		 */
		explicit UniquePtr(T* ptr = nullptr)
		: _ptr(ptr)
		{}

		/**
		 * @brief Assign the UniquePtr
		 *
		 * Effectively the same as calling `reset()`
		 */
		UniquePtr<T>& operator=(T* ptr) {
			reset(ptr);
			return *this;
		}

		/**
		 * @brief Copy constructor
		 *
		 * Construct a UniquePtr by transfering ownership from `u` to `*this`
		 * and making `other`'s pointer `nullptr`
		 */
		UniquePtr(UniquePtr<T>&& other) {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}

		/**
		 * @brief Move assignment operator
		 *
		 * Transfers ownership from `other` to `*this`
		 */
		UniquePtr<T>& operator=(UniquePtr<T>&& other) {
			reset(other.release());
			return *this;
		}

		/**
		 * @brief Destruct the managed object if one exists
		 */
		~UniquePtr() { reset(); }

		/**
		 * @brief Replace the managed object
		 */
		void reset(T* ptr = nullptr) {
			if (ptr == _ptr) return;

			T* old = _ptr;
			_ptr = ptr;
			if (old != nullptr) Deleter()(old);
		}

		/**
		 * @brief Get a pointer to the managed object
		 * 
		 * @return a pointer to the managed object if one exists, otherwise `nullptr`
		 */
		T* get() const { return _ptr; }

		/**
		 * @brief Release ownership of the managed object
		 *
		 * @return a pointer to the managed object
		 */
		T* release() { T* rv = _ptr; _ptr = nullptr; return rv; }

		/**
		 * @brief Dereference the pointer to the managed object
		 */
		T* operator->() const { return _ptr; }

		/**
		 * @brief Dereference the pointer to the managed object
		 */
		T& operator*() const { return *_ptr; }

		/**
		 * @brief Check if there is an associated managed object
		 *
		 * @return `true` if `*this` owns an object, `false` otherwise
		 */
        operator bool() const { return _ptr ? true : false; }

		// disable copy constructor and assignment operator
		UniquePtr(const UniquePtr<T>& other) = delete;
		UniquePtr<T>& operator=(const UniquePtr<T>& other) = delete;

	private:

		T* _ptr;
	};

}

#endif  // #ifndef _RNBO_UNIQUEPTR_H_
