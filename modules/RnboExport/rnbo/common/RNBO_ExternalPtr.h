//
//   RNBO_ExternalPtr.h
//   Created: 10 Feb 2016 2:46:28pm
//   Author:  Stefan Brunner
//
//

#ifndef _RNBO_EXTERNALPTR_H_
#define _RNBO_EXTERNALPTR_H_

#include "RNBO_UniquePtr.h"
#include "RNBO_ExternalBase.h"

namespace RNBO {

	/**
	 * @private
	 */
	template <typename T>
	class HolderPtr
	{
	public:
		HolderPtr()
		: _ptr(nullptr)
		{}

		HolderPtr(T* ptr) {
			_ptr.reset(ptr);
		}

		UniquePtr<T>& operator->() {
			return _ptr;
		}

		const UniquePtr<T>& operator->() const {
			return _ptr;
		}

		bool operator!()
		{
			return !_ptr;
		}

		T* get() const
		{
			return _ptr.get();
		}

		explicit operator const T*() const { return _ptr.get(); }

	private:
		UniquePtr<T> _ptr;
	};

	template <typename T1, typename T2>
	inline bool operator==(const T1* lhs, const HolderPtr<T2>& rhs)
	{
		return lhs == static_cast<const T1*>(rhs.get());
	}

	template <typename T1, typename T2>
	inline bool operator==(const HolderPtr<T2>& lhs, const T2* rhs)
	{
		return static_cast<const T2*>(lhs.get()) == rhs;
	}

	using ExternalPtr = HolderPtr<ExternalBase>;

}

#endif  // #ifndef _RNBO_EXTERNALPTR_H_
