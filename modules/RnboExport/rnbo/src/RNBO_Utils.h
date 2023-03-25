//
//  RNBO_Utils.h
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_Utils_H_
#define _RNBO_Utils_H_

#include <memory>

namespace RNBO {

	// implementation of make_unique()
	// taken from: https://isocpp.org/files/papers/N3656.txt
	// linked to from: http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding

	/**
	 * @private
	 */
	template<class T> struct _Unique_if {
		typedef std::unique_ptr<T> _Single_object;
	};

	/**
	 * @private
	 */
	template<class T> struct _Unique_if<T[]> {
		typedef std::unique_ptr<T[]> _Unknown_bound;
	};

	/**
	 * @private
	 */
	template<class T, size_t N> struct _Unique_if<T[N]> {
		typedef void _Known_bound;
	};

	/**
	 * @private
	 */
	template<class T, class... Args>
	typename _Unique_if<T>::_Single_object
	make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	/**
	 * @private
	 */
	template<class T>
	typename _Unique_if<T>::_Unknown_bound
	make_unique(size_t n) {
		typedef typename std::remove_extent<T>::type U;
		return std::unique_ptr<T>(new U[n]());
	}

	template<class T, class... Args>
	typename _Unique_if<T>::_Known_bound
	make_unique(Args&&...) = delete;


	// helper template prevents single-argument universal-reference constructor from acting like the copy constructor
	// see: http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
	template<typename A, typename B>
	using disable_if_same_or_derived =
	typename std::enable_if<!std::is_base_of<A,typename std::remove_reference<B>::type>::value>::type;


} // namespace RNBO


#endif // #ifndef _RNBO_Utils_H_
