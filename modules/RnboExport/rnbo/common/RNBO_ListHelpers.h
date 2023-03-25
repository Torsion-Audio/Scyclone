#ifndef _RNBO_LISTHELPERS_H_
#define _RNBO_LISTHELPERS_H_

// RNBO_LISTHELPERS.h -- list utilities used by generated code

#include "RNBO_Std.h"

namespace RNBO {

	ATTRIBUTE_UNUSED
	static list createListCopy(const list& l)
	{
		list tmp = l;
		return tmp;
	}

	/*
	* assume this statement:
	*
	* list newlist = oldlist;
	*
	* in C++ this will make a list copy, in JS this won't, so we need the below function (a NOOP in C++)
	*/
	ATTRIBUTE_UNUSED
	static const list& jsCreateListCopy(const list& l)
	{
		return l;
	}

	template <typename T> list serializeArrayToList(T *array, size_t size) {
		list result;
		result.reserve(size);
		for (size_t i = 0; i < size; i++) {
			result.push((number)(array[i]));
		}
		return result;
	}

	template <typename T> void deserializeArrayFromList(const list& l, T* result, size_t size) {
		for (size_t i = 0; i < size && i < l.length; i++) {
			result[i] = (T)(l[i]);
		}
	}

} // namespace RNBO

#endif // #ifndef _RNBO_LISTHELPERS_H_
