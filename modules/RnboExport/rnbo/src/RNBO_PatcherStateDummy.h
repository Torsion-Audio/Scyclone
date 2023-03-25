//
//  RNBO_PatcherStateDummy.h
//
//  Created by Jeremy Bernstein on 30.08.16.
//
//

#ifndef RNBO_PatcherStateDummy_h
#define RNBO_PatcherStateDummy_h

#include "RNBO_Types.h"
#include "RNBO_PatcherStateInterface.h"

namespace RNBO {

	/**
	 * @private
	 */
	class PatcherStateDummy : public PatcherStateInterface {

	private:

		void add(const char* key, number val) override { RNBO_UNUSED(key); RNBO_UNUSED(val); }
		void add(const char* key, Int val) override { RNBO_UNUSED(key); RNBO_UNUSED(val); }
		void add(const char* key, Index val) override { RNBO_UNUSED(key); RNBO_UNUSED(val); }
		void add(const char* key, bool val) override { RNBO_UNUSED(key); RNBO_UNUSED(val); }
		void add(const char* key, ExternalPtr& ext) override { RNBO_UNUSED(key); RNBO_UNUSED(ext); }
		void add(const char* key, PatcherEventTarget* patcherEventTarget) override { RNBO_UNUSED(key); RNBO_UNUSED(patcherEventTarget); }
		void add(const char* key, const list& theList) override { RNBO_UNUSED(key); RNBO_UNUSED(theList); }
		void add(const char* key, DataRef& dataRef) override { RNBO_UNUSED(key); RNBO_UNUSED(dataRef); }
		void add(const char* key, MultiDataRef& dataRef) override { RNBO_UNUSED(key); RNBO_UNUSED(dataRef); }
		void add(const char* key, signal sig) override { RNBO_UNUSED(key); RNBO_UNUSED(sig); }
		void add(const char* key, const char* str) override { RNBO_UNUSED(key); RNBO_UNUSED(str); }

		number getFloat(const char* key) override { RNBO_UNUSED(key); return number(0); }
		Int getInt(const char* key) override { RNBO_UNUSED(key); return Index(0); }
		Index getIndex(const char* key) override { RNBO_UNUSED(key); return Index(0); }
		bool getBool(const char* key) override { RNBO_UNUSED(key); return Index(0); }
		ExternalPtr getExternalPtr(const char* key) override { RNBO_UNUSED(key); return nullptr; }
		PatcherEventTarget* getEventTarget(const char* key) override { RNBO_UNUSED(key); return nullptr; }
		bool isEmpty() const override { return true; }
		list getList(const char* key) override { RNBO_UNUSED(key); return _list; }
		DataRef& getDataRef(const char *key) override { RNBO_UNUSED(key); return _dataRef; }
		MultiDataRef& getMultiDataRef(const char *key) override { RNBO_UNUSED(key); return _multiDataRef; }
		signal getSignal(const char *key) override { RNBO_UNUSED(key); return nullptr; }
		const char* getString(const char *key) override { RNBO_UNUSED(key); return ""; }

		bool containsValue(const char* key) const override { RNBO_UNUSED(key); return false; }

		PatcherStateInterface& getSubState(const char* key) override { RNBO_UNUSED(key); return *this; }
		PatcherStateInterface& getSubStateAt(const char* key, Index i) override { RNBO_UNUSED(key); RNBO_UNUSED(i); return *this; }
		const PatcherStateInterface& getSubState(const char* key) const override { RNBO_UNUSED(key); return *this; }
		const PatcherStateInterface& getSubStateAt(const char* key, Index i) const override { RNBO_UNUSED(key); RNBO_UNUSED(i); return *this; }

		list				_list;
		DataRef				_dataRef;
		MultiDataRef		_multiDataRef;
	};

}

#endif // RNBO_PatcherStateDummy_h
