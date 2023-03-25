
#ifndef _RNBO_PatcherstateInterface_H_
#define _RNBO_PatcherstateInterface_H_

#include "RNBO_Types.h"
#include "RNBO_ExternalPtr.h"
#include "RNBO_List.h"

namespace RNBO {

	class PatcherEventTarget;
	class DataRef;
	class MultiDataRef;

	constexpr const char *voiceKey = "XX_##_Voices_##_XX";
	constexpr const char *eventTargetKey = "XX_##_PatcherEventTarget_##_XX";
	constexpr const char *patcherSerialKey = "XX_##_PatcherSerial_##_XX";

	/**
	 * @private
	 */
	template <typename U> class StateHelper {
	public:
		StateHelper(const char* key, U& state)
		: _key(key)
		, _state(state)
		{}

		template <typename T> void operator=(const T& val) {
			_state.add(_key, val);
		}

		template <typename T> T& operator=(T&& val) {
			_state.add(_key, val);
			return val;
		}

		void operator=(ExternalPtr& val) {
			_state.add(_key, val);
		}

		operator number() { return _state.getFloat(_key); }

		explicit operator Int() { return _state.getInt(_key); }
		explicit operator Index() { return _state.getIndex(_key); }
		explicit operator bool() { return _state.getBool(_key); }
		explicit operator signal() { return _state.getSignal(_key); }
		explicit operator PatcherEventTarget*() { return _state.getEventTarget(_key); }

		operator ExternalPtr() { return _state.getExternalPtr(_key); }
		operator U&() { return _state.getSubState(_key); }
		operator list() { return _state.getList(_key); }
		operator DataRef&() { return _state.getDataRef(_key); }
		operator MultiDataRef&() { return _state.getMultiDataRef(_key); }

		U& operator[](Index i) {
			return _state.getSubStateAt(_key, i);
		}

		bool containsValue() const { return _state.containsValue(_key); }

	protected:
		const char*	_key;
		U& _state;
	};

	/**
	 * @private
	 */
	class PatcherStateInterface {
	public:

		virtual ~PatcherStateInterface() {}

		StateHelper<PatcherStateInterface> operator[](const char* key) {
			StateHelper<PatcherStateInterface> helper(key, *this);
			return helper;
		}

		virtual PatcherStateInterface& getSubState(const char* key) = 0;
		virtual PatcherStateInterface& getSubStateAt(const char* key, Index i) = 0;
		virtual const PatcherStateInterface& getSubState(const char* key) const = 0;
		virtual const PatcherStateInterface& getSubStateAt(const char* key, Index i) const = 0;

		virtual bool isEmpty() const = 0;

	private:

		friend class StateHelper<PatcherStateInterface>;

		virtual void add(const char* key, number val) = 0;
		virtual void add(const char* key, Int val) = 0;
		virtual void add(const char* key, Index val) = 0;
		virtual void add(const char* key, bool val) = 0;
		virtual void add(const char* key, ExternalPtr& ext) = 0;
		virtual void add(const char* key, PatcherEventTarget* patcherEventTarget) = 0;
		virtual void add(const char* key, const list& theList) = 0;
		virtual void add(const char * key, DataRef& dataRef) = 0;
		virtual void add(const char * key, MultiDataRef& dataRef) = 0;
		virtual void add(const char * key, signal sig) = 0;
		virtual void add(const char * key, const char* str) = 0;

		virtual number getFloat(const char* key) = 0;
		virtual Int getInt(const char* key) = 0;
		virtual Index getIndex(const char* key) = 0;
		virtual bool getBool(const char* key) = 0;
		virtual ExternalPtr getExternalPtr(const char* key) = 0;
		virtual PatcherEventTarget* getEventTarget(const char* key) = 0;
		virtual list getList(const char* key) = 0;
		virtual DataRef& getDataRef(const char *key) = 0;
		virtual MultiDataRef& getMultiDataRef(const char *key) = 0;
		virtual signal getSignal(const char *key) = 0;
		virtual const char* getString(const char *key) = 0;

		virtual bool containsValue(const char* key) const = 0;
	};

	ATTRIBUTE_UNUSED
	static inline bool containsValue(StateHelper<PatcherStateInterface> helper)
	{
		return helper.containsValue();
	}

	ATTRIBUTE_UNUSED
	static PatcherStateInterface& getSubState(PatcherStateInterface& state, const char* key)
	{
		return state.getSubState(key);
	}

	ATTRIBUTE_UNUSED
	static PatcherStateInterface& getSubStateAt(PatcherStateInterface& state, const char* key, Index i)
	{
		return state.getSubStateAt(key, i);
	}

	ATTRIBUTE_UNUSED
	static bool stateIsEmpty(const PatcherStateInterface& state)
	{
		return state.isEmpty();
	}

} // namespace RNBO

#endif  // _RNBO_PatcherstateInterface_H_
