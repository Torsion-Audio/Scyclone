//
//  RNBO_PatcherState.cpp
//  Created: 17 Feb 2016 2:02:52pm
//  Author:  stb
//
//

#include "RNBO_PatcherState.h"

namespace RNBO {

	ValueHolder::ValueHolder(number val)
	: _value(val)
	{}

	ValueHolder::ValueHolder(Int val)
	: _value((Int)val)
	{}

	ValueHolder::ValueHolder(Index val)
	: _value(val)
	{}

	ValueHolder::ValueHolder(bool val)
	: _value(val)
	{}

	ValueHolder::ValueHolder(ExternalPtr& ext)
	: _value(std::move(ext))
	{}

	ValueHolder::ValueHolder(StateMapPtr substate)
	: _value(substate)
	{}

	ValueHolder::ValueHolder(PatcherEventTarget* patcherEventTarget)
	: _value(patcherEventTarget)
	{}

	ValueHolder::ValueHolder(const list& theList)
	: _value(theList)
	{}

	ValueHolder::ValueHolder(DataRef&& dataRef)
	: _value(std::move(dataRef))
	{}

	ValueHolder::ValueHolder(MultiDataRef&& dataRef)
	: _value(std::move(dataRef))
	{}

	ValueHolder::ValueHolder(const signal sig)
	: _value(sig)
	{}

	ValueHolder::ValueHolder(const char* str)
	: _value(String(str))
	{}

	ValueHolder::ValueHolder()
	{}

	ValueHolder::operator number() const { return mpark::get<number>(_value); }
	ValueHolder::operator Int() const { return mpark::get<Int>(_value); }
	ValueHolder::operator Index() const { return mpark::get<Index>(_value); }
	ValueHolder::operator bool() const { return mpark::get<bool>(_value); }
	ValueHolder::operator PatcherEventTarget*() { return mpark::get<PatcherEventTarget*>(_value); }
	ValueHolder::operator signal() { return mpark::get<signal>(_value); }

	ValueHolder::operator ExternalPtr() { return std::move(mpark::get<ExternalPtr>(_value)); }
	ValueHolder::operator const list&() const { return mpark::get<list>(_value); }
	ValueHolder::operator DataRef&() { return mpark::get<DataRef>(_value); }
	ValueHolder::operator MultiDataRef&() { return mpark::get<MultiDataRef>(_value); }
	ValueHolder::operator const char*() const { return mpark::get<String>(_value).c_str(); }

	ValueHolder::operator PatcherState&() {
		if (!mpark::holds_alternative<StateMapPtr>(_value)) allocateSubState();
		return *mpark::get<StateMapPtr>(_value);
	}
	PatcherState& ValueHolder::operator[](Index i) {
		if (!mpark::holds_alternative<SubStateMapPtr>(_value)) allocateSubStateMap();
		std::shared_ptr<ValueHolder> ps = (*mpark::get<SubStateMapPtr>(_value))[i];
		if (!ps) {
			ps = std::make_shared<ValueHolder>();
			(*mpark::get<SubStateMapPtr>(_value))[i] = ps;
		}
		return *(*mpark::get<SubStateMapPtr>(_value))[i];
	}

	ValueHolder::operator const PatcherState&() const
	{
		return *mpark::get<StateMapPtr>(_value);
	}

	const PatcherState& ValueHolder::operator[](Index i) const
	{
		std::shared_ptr<ValueHolder> ps = mpark::get<SubStateMapPtr>(_value)->at(i);
		if (!ps) {
			ps = std::make_shared<ValueHolder>();
			mpark::get<SubStateMapPtr>(_value)->at(i) = ps;
		}
		return *mpark::get<SubStateMapPtr>(_value)->at(i);
	}

	Index ValueHolder::getSubStateMapSize() const
	{
		return mpark::holds_alternative<SubStateMapPtr>(_value) ? mpark::get<SubStateMapPtr>(_value)->size() : 0;
	}

	void ValueHolder::allocateSubState()
	{
		RNBO_ASSERT(getType() == ValueHolder::NONE);
		_value = std::make_shared<PatcherState>();
	}

	void ValueHolder::allocateSubStateMap()
	{
		RNBO_ASSERT(getType() == ValueHolder::NONE);
		_value = std::make_shared<SubStateMapType>();
	}

	struct GetTypeVisitor {
		constexpr ValueHolder::Type operator()(const mpark::monostate&) const { return ValueHolder::NONE; }
		constexpr ValueHolder::Type operator()(const number&) const { return ValueHolder::NUMBER; }
		constexpr ValueHolder::Type operator()(const Int&) const { return ValueHolder::INTVALUE; }
		constexpr ValueHolder::Type operator()(const Index&) const { return ValueHolder::INDEX; }
		constexpr ValueHolder::Type operator()(const bool&) const { return ValueHolder::BOOLEAN; }
		constexpr ValueHolder::Type operator()(const ExternalPtr&) const { return ValueHolder::EXTERNAL; }
		constexpr ValueHolder::Type operator()(const StateMapPtr&) const { return ValueHolder::SUBSTATE; }
		constexpr ValueHolder::Type operator()(const PatcherEventTarget&) const { return ValueHolder::EVENTTARGET; }
		constexpr ValueHolder::Type operator()(const SubStateMapPtr&) const { return ValueHolder::SUBSTATEMAP; }
		constexpr ValueHolder::Type operator()(const list&) const { return ValueHolder::LIST; }
		constexpr ValueHolder::Type operator()(const DataRef&) const { return ValueHolder::DATAREF; }
		constexpr ValueHolder::Type operator()(const MultiDataRef&) const { return ValueHolder::MULTIREF; }
		constexpr ValueHolder::Type operator()(const signal&) const { return ValueHolder::SIGNAL; }
		constexpr ValueHolder::Type operator()(const String&) const { return ValueHolder::STRING; }
	};

	ValueHolder::Type ValueHolder::getType() const
	{
		return mpark::visit(GetTypeVisitor{}, _value);
	}

	void PatcherState::add(const char* key, number val)
	{
		_map.emplace(key, val);
	}

	void PatcherState::add(const char* key, Int val)
	{
		_map.emplace(key, val);
	}

	void PatcherState::add(const char* key, Index val)
	{
		_map.emplace(key, val);
	}

	void PatcherState::add(const char* key, bool val)
	{
		_map.emplace(key, val);
	}

	void PatcherState::add(const char* key, ExternalPtr& ext)
	{
		_map.emplace(key, ext);
	}

	void PatcherState::add(const char* key, PatcherEventTarget* patcherEventTarget)
	{
		_map.emplace(key, patcherEventTarget);
	}

	void PatcherState::add(const char* key, const list& theList)
	{
		_map.emplace(key, theList);
	}

	void PatcherState::add(const char* key, DataRef& dataRef)
	{
		_map.emplace(key, std::move(dataRef));
	}

	void PatcherState::add(const char* key, MultiDataRef& dataRef)
	{
		_map.emplace(key, std::move(dataRef));
	}

	void PatcherState::add(const char* key, signal sig)
	{
		_map.emplace(key, sig);
	}

	void PatcherState::add(const char* key, const char* str)
	{
		_map.emplace(key, str);
	}

	number PatcherState::getFloat(const char* key)
	{
		return (number)_map[key];
	}

	Int PatcherState::getInt(const char* key)
	{
		return static_cast<Int>(_map[key]);
	}

	Index PatcherState::getIndex(const char* key)
	{
		return static_cast<Index>(_map[key]);
	}

	bool PatcherState::getBool(const char* key)
	{
		return static_cast<bool>(_map[key]);
	}

	ExternalPtr PatcherState::getExternalPtr(const char* key)
	{
		return _map[key];
	}

	PatcherEventTarget* PatcherState::getEventTarget(const char* key)
	{
		return (PatcherEventTarget*)_map[key];
	}

	PatcherState& PatcherState::getSubState(const char* key)
	{
		return _map[key];
	}

	PatcherState& PatcherState::getSubStateAt(const char* key, Index i)
	{
		return _map[key][i];
	}

	const PatcherState& PatcherState::getSubState(const char* key) const
	{
		return _map.at(key);
	}

	const PatcherState& PatcherState::getSubStateAt(const char* key, Index i) const
	{
		return _map.at(key)[i];
	}

	bool PatcherState::isEmpty() const {
		return _map.empty();
	}

	list PatcherState::getList(const char* key)
	{
		const ValueHolder& value = _map[key];
		const list& list = value;
		return list;
	}

	DataRef& PatcherState::getDataRef(const char* key)
	{
		return _map[key];
	}

	MultiDataRef& PatcherState::getMultiDataRef(const char* key)
	{
		return _map[key];
	}

	signal PatcherState::getSignal(const char* key)
	{
		return (signal)_map[key];
	}

	const char* PatcherState::getString(const char* key)
	{
		return _map[key];
	}

	bool PatcherState::containsValue(const char* key) const
	{
		return _map.find(key) != _map.end();
	}
}
