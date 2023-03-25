#include <RNBO.h>
#include <RNBO_MaxPresetAdapter.h>
#include <RNBO_MidiStreamParser.h>
#include <json/json.hpp>
#include <c74_min.h>
#include <atomic>
#include <optional>
#include <set>
#include <regex>

#ifdef RNBO_INCLUDE_DESCRIPTION_FILE
#include <rnbo_description.h>
using RNBO::patcher_description;
#endif

//rnbo_bufferref doesn't use namespaced style symbols, so expose them as needed here
using c74::max::t_symbol;
using c74::max::t_max_err;
using c74::max::t_buffer_ref;
using c74::max::t_object;
#include <adapters/max/rnbo_bufferref.h>

using RNBO::Index;
using RNBO::ParameterIndex;
using RNBO::ParameterInfo;
using RNBO::ParameterType;
using RNBO::ParameterValue;
using RNBO::ConstByteArray;
using RNBO::IOType;
using RNBO::DataRefIndex;
using RNBO::ConstRefList;
using RNBO::UpdateRefCallback;
using RNBO::ReleaseRefCallback;
using RNBO::ExternalDataHandler;
using RNBO::ExternalDataIndex;
using RNBO::ExternalDataRef;
using RNBO::ExternalDataId;
using RNBO::MessageEvent;
using RNBO::MidiEvent;
using RNBO::TAG;
using RNBO::SampleValue;
using RNBO::MillisecondTime;

using c74::min::endl;
using c74::min::message_type;
using c74::min::flags;

namespace {
	const std::regex inportEventInRegex("^in[[:digit:]]+$");
}


//attributes are thread safe as they just read/write parameters, which are thread safe in RNBO
typedef c74::min::attribute<ParameterValue, c74::min::threadsafe::yes> attr_param;
typedef c74::min::attribute<c74::min::symbol, c74::min::threadsafe::yes> attr_enum_param;
typedef c74::min::attribute<c74::min::symbol, c74::min::threadsafe::no> attr_buffer;
typedef c74::min::attribute<c74::min::symbol, c74::min::threadsafe::no> attr_transport;

class LockGuard;
class Mutex {
	public:
		Mutex(long flags = 0) {
			c74::max::systhread_mutex_new(&mMutex, flags);
		}
		~Mutex() {
			c74::max::systhread_mutex_free(mMutex);
		}
	private:
		c74::max::t_systhread_mutex mMutex;
		friend LockGuard;
};

class LockGuard {
	public:
		LockGuard(Mutex& m) {
			mMutex = m.mMutex;
			c74::max::systhread_mutex_lock(mMutex);
		}
		~LockGuard() {
			c74::max::systhread_mutex_unlock(mMutex);
		}
	private:
		c74::max::t_systhread_mutex mMutex;
};

class Sync {
	private:
		c74::min::object_base * mOwner;
		c74::max::t_itm* mCachedItm = nullptr;
		c74::min::symbol mTransport;
		c74::min::symbol mInternalClock;
		c74::min::symbol mSymFree;
		c74::min::symbol mSymTempo;
		double mLastTempo = -1.0;
		int mLastTransport = -1.0;
		double mLastBeatTime = -1.0;
		long mLastNumerator = -1.0;
		long mLastDenominator = -1.0;

		void freeCache() {
			auto c = mCachedItm;
			mCachedItm = nullptr;
			if (c) {
				c74::max::object_detach_byptr(mOwner->maxobj(), c);
				c74::max::itm_dereference(c);
			}
		}
	public:
		Sync(c74::min::object_base * owner) : mOwner(owner), mInternalClock("internal"), mSymFree("free"), mSymTempo("tempo") {
		}

		~Sync() {
			freeCache();
		}

		c74::min::symbol transportName() const {
			return mTransport.empty() ? mInternalClock : mTransport;
		}

		bool updateTransport(c74::min::symbol name) {
			if (mTransport != name) {
				mTransport = name;
				updateCache();
				return true;
			}
			return false;
		}

		bool handleNotify(void * sender, c74::min::symbol msg) {
			//XXX should we return true even if msg doesn't == free?
			if (sender == mCachedItm && msg == mSymFree) {
				mCachedItm = nullptr;
				return true;
			}
			return false;
		}

		void updateCache() {
			freeCache();
			auto o = mOwner->maxobj();
			auto itm = (c74::max::t_itm *)itm_getfromarg(o, mTransport);
			c74::max::object_attach_byptr_register(o, itm, c74::max::CLASS_NOBOX);
			c74::max::itm_reference(itm);
			mCachedItm = itm;
		}

		void updateTime(MillisecondTime scheduleNow, std::function<void(RNBO::EventVariant)> scheduleEvent) {
			auto itm = mCachedItm;
			if (!itm)
				return;
			double tempo = c74::max::itm_gettempo(itm);
			long transportState = c74::max::itm_getstate(itm);
			double beattime = c74::max::itm_getticks(itm) / 480.;

			long numerator = -1;
			long denominator = -1;
			c74::max::itm_gettimesignature(itm, &numerator, &denominator);

			if (tempo != -1 && tempo != mLastTempo) {
				RNBO::TempoEvent event(scheduleNow, tempo);
				scheduleEvent(event);
				mLastTempo = tempo;
			}
			if (transportState != -1 && transportState != mLastTransport) {
				RNBO::TransportEvent event(scheduleNow, transportState ? RNBO::TransportState::RUNNING : RNBO::TransportState::STOPPED);
				scheduleEvent(event);
				mLastTransport = transportState;
			}
			if (beattime != -1 && beattime != mLastBeatTime) {
				RNBO::BeatTimeEvent event(scheduleNow, beattime);
				scheduleEvent(event);
				mLastBeatTime = beattime;
			}
			if (
					(numerator != -1 && numerator != mLastNumerator) ||
					(denominator != -1 && denominator != mLastDenominator)
				 ) {
				RNBO::TimeSignatureEvent event(scheduleNow, numerator, denominator);
				scheduleEvent(event);
				mLastNumerator = numerator;
				mLastDenominator = denominator;
			}
		}

		void withItm(std::function<void(c74::max::t_itm*)> f) {
			//XXX should we lock around mCachedItm ??
			auto itm = mCachedItm;
			if (itm) {
				f(itm);
			}
		}

		void setRunning(bool v) {
			withItm([v](c74::max::t_itm* itm) {
				if (v) {
					itm_resume(itm);
				} else {
					itm_pause(itm);
				}
			});
		}

		void setTempo(double v) {
			withItm([v, this](c74::max::t_itm* itm) {
				object_attr_setfloat(itm, mSymTempo, v);
			});
		}

		void setBeatTime(double v) {
			withItm([v](c74::max::t_itm* itm) {
				//XXX fixed 480 PPQ?
				itm_seek(itm, itm_getticks(itm), v * 480, false);
			});
		}

		void setTimeSignature(double n, double d) {
			withItm([n, d](c74::max::t_itm* itm) {
				itm_settimesignature(itm, n, d, 0);
			});
		}
};

//data handler, provides max buffers to RNBO
class MaxExternalDataHandler : public RNBO::ExternalDataHandler {
	private:
		std::vector<t_rnbo_bufferref *> mBuffers;
		std::vector<std::unique_ptr<attr_buffer>> mAttributes;

		//iterator helper
		void each(DataRefIndex numRefs, ConstRefList refList, std::function<void(ExternalDataIndex, const ExternalDataRef* const, t_rnbo_bufferref *)> f) {
			for (ExternalDataIndex i = 0; i < std::min((ExternalDataIndex)mBuffers.size(), (ExternalDataIndex)numRefs); i++) {
				f(i, refList[i], mBuffers[i]);
			}
		}
	public:
		MaxExternalDataHandler(c74::min::object_base * owner, RNBO::CoreObject& rnbo) {
			//register the class, will not double register.
			//min creates an object before it sets up the class so we regsiter on demand
			rnbo_bufferref_register();
			ExternalDataIndex count = rnbo.getNumExternalDataRefs();
			for (ExternalDataIndex i = 0; i < count; i++) {
				ExternalDataId memoryId = rnbo.getExternalDataId(i);
				auto id = c74::min::symbol(c74::max::gensym(memoryId));

				auto b = rnbo_bufferref_new(c74::min::k_sym__empty);
				mBuffers.emplace_back(b);
				mAttributes.emplace_back(std::make_unique<attr_buffer>(
						owner,
						id,
						rnbo_bufferref_getname(b),
						c74::min::setter {
							[b](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
								c74::min::symbol in = args[0];
								rnbo_bufferref_setname(b, in);
								return { };
							}
						},
						c74::min::getter {
							[b]() -> c74::min::atoms {
								return { rnbo_bufferref_getname(b) };
							}
						}
				));
			}
			rnbo.setExternalDataHandler(this);
		}

		~MaxExternalDataHandler() {
			for (auto b: mBuffers) {
				c74::max::object_free(b);
			}
		}

		//TODO required anymore?
		c74::min::atoms handle_notification(c74::min::object_base * owner, const c74::min::atoms& args) {
			return {};
		}

		virtual void processBeginCallback(DataRefIndex numRefs, ConstRefList refList, UpdateRefCallback updateDataRef, ReleaseRefCallback releaseDataRef) override {
			//iterate over the references and lock the buffers if possible
			each(numRefs, refList, [updateDataRef, releaseDataRef](ExternalDataIndex i, const ExternalDataRef* const ref, t_rnbo_bufferref * bufr) {
					DataRefBindMaxBuffer(i, ref, bufr, updateDataRef, releaseDataRef);
			});
		}

		virtual void processEndCallback(DataRefIndex numRefs, ConstRefList refList) override {
			//iterate, mark buffers dirty if needed and unlock
			each(numRefs, refList, [](ExternalDataIndex i, const ExternalDataRef* const ref, t_rnbo_bufferref * bufr) {
					DataRefUnbindMaxBuffer(ref, bufr);
			});
		}
};

class MaxExternalEventHandler : public RNBO::EventHandler {
	public:
		typedef std::function<void(MessageEvent)> MessageEventCallback;
		typedef std::function<void(MidiEvent)> MidiEventCallback;
		typedef std::function<void(RNBO::TransportEvent)> TransportEventCallback;
		typedef std::function<void(RNBO::TempoEvent)> TempoEventCallback;
		typedef std::function<void(RNBO::BeatTimeEvent)> BeatTimeEventCallback;
		typedef std::function<void(RNBO::TimeSignatureEvent)> TimeSignatureEventCallback;
		typedef std::function<void(const RNBO::ParameterIndex)> ParameterEventCallback;
		typedef std::function<void()> PresetTouchedCallback;

		MaxExternalEventHandler(
				c74::min::object_base * owner,
				RNBO::CoreObject& rnbo,
				RNBO::ScheduleCallback scheduleCallback,
				MessageEventCallback mc,
				MidiEventCallback mec,

				TransportEventCallback transportCallback,
				TempoEventCallback tempoCallback,
				BeatTimeEventCallback beatTimeCallback,
				TimeSignatureEventCallback timeSigCallback,

				ParameterEventCallback pc,
				PresetTouchedCallback pt
				) :
			mTimer(owner,
				MIN_FUNCTION {
					drainEvents();
					return { 0 };
				}),
			mMessageCallback(mc),
			mMidiCallback(mec),

			mTransportCallback(transportCallback),
			mTempoCallback(tempoCallback),
			mBeatTimeCallback(beatTimeCallback),
			mTimeSigCallback(timeSigCallback),

			mParameterCallback(pc),
			mPresetTouchedCallback(pt)

		{
			if (scheduleCallback) {
				mSyncParameterInterface = rnbo.createParameterInterface(RNBO::ParameterEventInterface::Trigger, this);
				mParameterInterface = rnbo.createParameterInterface(RNBO::ParameterEventInterface::MultiProducer, nullptr);
				mSyncParameterInterface->setScheduleCallback(scheduleCallback);
			} else {
				mParameterInterface = rnbo.createParameterInterface(RNBO::ParameterEventInterface::MultiProducer, this);
			}
		}

		void eventsAvailable() override {
			mTimer.delay(0);
		}

		void scheduleEvent(RNBO::EventVariant event) {
			pinterface()->scheduleEvent(event);
		}

		void scheduleTriggerEvent(MillisecondTime time) {
			if (mSyncParameterInterface) {
				const RNBO::UniversalEvent event(time, -1, 0, nullptr);
				mSyncParameterInterface->scheduleEvent(event);
			}
		}

		void setParameterValue(RNBO::ParameterIndex i, double v) {
			return pinterface()->setParameterValue(i, v);
		}

		double getParameterValue(RNBO::ParameterIndex i) {
			return pinterface()->getParameterValue(i);
		}

		void sendMessage(RNBO::MessageTag tag, RNBO::number payload, RNBO::MessageTag objectId = 0, MillisecondTime eventTime = RNBO::RNBOTimeNow) {
			pinterface()->sendMessage(tag, payload, objectId, eventTime);
		}

		void sendMessage(RNBO::MessageTag tag, RNBO::UniqueListPtr payload, RNBO::MessageTag objectId = 0, MillisecondTime eventTime = RNBO::RNBOTimeNow) {
			pinterface()->sendMessage(tag, std::move(payload), objectId, eventTime);
		}

		void sendMessage(RNBO::MessageTag tag, RNBO::MessageTag objectId = 0, MillisecondTime eventTime = RNBO::RNBOTimeNow) {
			pinterface()->sendMessage(tag, objectId, eventTime);
		}

		void handlePresetEvent(const RNBO::PresetEvent& pe) override {
			if (mPresetTouchedCallback && pe.getType() == RNBO::PresetEvent::Touched) {
				mPresetTouchedCallback();
			}
		}

		void handleParameterEvent(const RNBO::ParameterEvent& event) override {
			if (mParameterCallback)
				mParameterCallback(event.getIndex());
		}

		void handleMessageEvent(const MessageEvent& event) override {
			if (mMessageCallback)
				mMessageCallback(event);
		}

		void handleMidiEvent(const MidiEvent& event) override {
			if (mMidiCallback)
				mMidiCallback(event);
		}

		void handleTransportEvent(const RNBO::TransportEvent& e) override
		{
			if (mTransportCallback)
				mTransportCallback(e);
		}

		void handleTempoEvent(const RNBO::TempoEvent& e) override
		{
			if (mTempoCallback)
				mTempoCallback(e);
		}

		void handleBeatTimeEvent(const RNBO::BeatTimeEvent& e) override
		{
			if (mBeatTimeCallback)
				mBeatTimeCallback(e);
		}

		void handleTimeSignatureEvent(const RNBO::TimeSignatureEvent& e) override
		{
			if (mTimeSigCallback)
				mTimeSigCallback(e);
		}


	private:
		RNBO::ParameterEventInterfaceUniquePtr& pinterface() {
			if (mSyncParameterInterface && c74::max::systhread_istimerthread())
				return mSyncParameterInterface;
			return mParameterInterface;
		}
		c74::min::timer<c74::min::timer_options::deliver_on_scheduler> mTimer;
		RNBO::ParameterEventInterfaceUniquePtr mParameterInterface;
		RNBO::ParameterEventInterfaceUniquePtr mSyncParameterInterface; //only for max for live
		MessageEventCallback mMessageCallback;
		MidiEventCallback mMidiCallback;
		TransportEventCallback mTransportCallback;
		TempoEventCallback mTempoCallback;
		BeatTimeEventCallback mBeatTimeCallback;
		TimeSignatureEventCallback mTimeSigCallback;

		ParameterEventCallback mParameterCallback;
		PresetTouchedCallback mPresetTouchedCallback;
};

extern "C"
void rnbowrapper_parameter_attr_override(void *_x, t_object *valueof)
{
	auto sym_long = c74::max::gensym("long");
	c74::max::object_attr_addattr_parse(valueof, "parameter_initial_enable", "invisible", sym_long, 0, "1");
	c74::max::object_attr_addattr_parse(valueof, "parameter_initial", "invisible", sym_long, 0, "1");
	c74::max::object_attr_setlong(valueof, c74::max::gensym("parameter_initial_editable"), 0);
	c74::max::object_attr_addattr_parse(valueof, "parameter_initial", "inspector", sym_long, 0, "0");
	c74::max::object_attr_addattr_parse(valueof, "parameter_invisible", "invisible", sym_long, 0, "1");
}

class rnbo_external_wrapper :
	public c74::min::object<rnbo_external_wrapper>
#if RNBO_WRAPPER_HAS_AUDIO
	, public c74::min::vector_operator<>
#endif
{
	//don't generate max ref
	MIN_FLAGS { c74::min::documentation_flags::do_not_generate };
	public:
		virtual ~rnbo_external_wrapper() {
			//make sure the process method isn't currently running
			LockGuard guard(mDSPStateMutex);
			//stop the non audio thread processor
			mProcessClock.stop();
			//destroy buffers before killing notify, because we get a callback
			mDataHandler.reset();
			mNotify.reset();
		}

		rnbo_external_wrapper(const c74::min::atoms& args = {}) :
			mProcessClock(this,
				[this](const c74::min::atoms& _args, const int _inlet) -> c74::min::atoms {
					process_nodsp();
					return { };
				}),
			mEventOutClock(this,
				[this](const c74::min::atoms& _args, const int _inlet) -> c74::min::atoms {
					send_queued_events();
					return { };
				})
		{
			//some behaviors are slightly different in live
			auto p = c74::max::gensym("#P");
			if (p) {
				auto patcher = (t_object *)p->s_thing;
				if (patcher)
					mIsInLive = c74::max::object_method(patcher, c74::max::gensym("isinlive"));
			}

			//handle inlets and outlets
			auto it = [](long count, std::string prefix, std::function<void(std::string)> fn) {
				for (long i = 0; i < count; i++) {
					std::string n = prefix;
					if (count != 1)
						n += " " + std::to_string(i + 1);
					fn(n);
				}
			};

			bool add_inlets = true;
			bool add_outlets = true;
			bool has_inlets = false;

#ifdef RNBO_INCLUDE_DESCRIPTION_FILE
			std::set<std::string> outletNames;
			try {
				if (patcher_description.contains("inlets")) {
					auto inlets = patcher_description["inlets"];
					if (inlets.is_array()) {
						for (auto i: inlets) {
							if (!(i.contains("index") && i.contains("tag") && i.contains("type"))) {
								continue;
							}
							auto t = i["type"].get<std::string>();
							auto tag = i["tag"].get<std::string>();
							auto index = std::to_string(i["index"].get<int>());
							std::string name;
							if (i.contains("comment")) {
								name = i["comment"].get<std::string>();
							}
							if (t == "signal") {
								//setup signal map
								mSignalInletsMap.push_back(mInlets.size());
								mSignalInlets.push_back(nullptr);

								if (name.size() == 0) {
									name = "Signal Inlet #" + index;
								}
								mInlets.emplace_back(std::make_unique<c74::min::inlet<>>(this, name, "signal"));
							} else if (t == "event") {
								if (name.size() == 0) {
									name = "Message Inlet #" + index;
								}
								//to lookup later
								mMessageInletMap.emplace(std::make_pair(mInlets.size(), c74::max::gensym(tag.c_str())));
								mInlets.emplace_back(std::make_unique<c74::min::inlet<>>(this, name));
							} else if (t == "midi") {
								//skip
							} else {
								cerr << "unknown iolet type " << t << " for tag: " << tag << c74::min::endl;
								continue;
							}
							has_inlets = true;
						}
						add_inlets = false;
					}
				}
			} catch (std::exception& e) {
				cerr << "exception processing inlet json" << c74::min::endl;
			}

			try {
				if (patcher_description.contains("outlets")) {
					auto outlets = patcher_description["outlets"];
					if (outlets.is_array()) {
						for (auto i: outlets) {
							if (!(i.contains("index") && i.contains("tag") && i.contains("type"))) {
								continue;
							}
							auto t = i["type"].get<std::string>();
							auto tag = i["tag"].get<std::string>();
							auto index = std::to_string(i["index"].get<int>());
							std::string name;
							if (i.contains("comment")) {
								name = i["comment"].get<std::string>();
							}
							if (t == "signal") {
								if (name.size() == 0) {
									name = "Signal Outlet #" + index;
								}
								mOutlets.emplace_back(std::make_unique<c74::min::outlet<>>(this, name, "signal"));
							} else if (t == "event") {
								if (name.size() == 0) {
									name = "Message Outlet #" + index;
								}
								//push the index
								mMessageOutletMap.emplace(std::make_pair(c74::max::gensym(tag.c_str()), mOutlets.size()));
								mOutlets.emplace_back(std::make_unique<c74::min::outlet<>>(this, name));
							} else if (t == "midi") {
								//skip
							} else {
								cerr << "unknown iolet type " << t << " for tag: " << tag << c74::min::endl;
								continue;
							}
							outletNames.insert(tag);
						}
						add_outlets = false;
					}
				}
			} catch (std::exception& e) {
				cerr << "exception processing outlet json" << c74::min::endl;
			}

			try {
				if (patcher_description.contains("inports")) {
					auto inports = patcher_description["inports"];
					for (auto i: inports) {
						if (!i.contains("tag")) {
							continue;
						}
						std::string name = i["tag"].get<std::string>();
						if (std::regex_match(name, inportEventInRegex)) {
							continue;
						}
						mInportMessages.emplace_back(std::make_unique<c74::min::message<>>(this, name,
							[this, name](const c74::min::atoms& args, const int _inlet) -> c74::min::atoms {
								//add the selector (as a symbol) and handle
								c74::min::atoms withSelector = { c74::min::symbol(name.c_str()) };
								for (auto a: args) {
									withSelector.push_back(a);
								}
								handleInportMessage(withSelector, name);
								return {};
							})
						);
					}
				}
			} catch (std::exception& e) {
				cerr << "exception processing inport json" << c74::min::endl;
			}

#endif

			if (add_inlets) {
				it(mRNBOObj.getNumInputChannels(), "Signal Inlet", [this, &has_inlets](std::string name) {
					mSignalInletsMap.push_back(mInlets.size());
					mSignalInlets.push_back(nullptr);
					mInlets.emplace_back(std::make_unique<c74::min::inlet<>>(this, name, "signal"));
					has_inlets = true;
				});
			}

			//add a dummy, msg/attribute inlet if there are not any inlets, so the midi inlet is the 2nd inlet
			if (!has_inlets)
				mInlets.emplace_back(std::make_unique<c74::min::inlet<>>(this, "Attribute Inlet"));

			//offset, could just use a map?
			mMidiProcessStart = mInlets.size();
			it(mRNBOObj.getNumMidiInputPorts(), "Midi Inlet", [this](std::string name) {
				mInlets.emplace_back(std::make_unique<c74::min::inlet<>>(this, name));
				mMidiProcess.emplace_back(std::make_unique<RNBO::MidiStreamParser>());
			});

			if (add_outlets) {
				it(mRNBOObj.getNumOutputChannels(), "Signal Outlet", [this](std::string name) {
					mOutlets.emplace_back(std::make_shared<c74::min::outlet<>>(this, name, "signal"));
				});
			}

			//XXX only one for now
			if (mRNBOObj.getNumMidiOutputPorts()) {
				mMIDIOutlet = std::make_shared<c74::min::outlet<>>(this, "Midi Outlet");
				mOutlets.emplace_back(mMIDIOutlet);
			}

			mNotify = std::make_unique<c74::min::message<>>(this, "notify",
				[this](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
					if (mSync && (args.size() == 5)) {
						c74::min::notification n { args };
						if (mSync->handleNotify(n.source(), n.name()))
							return {};
					}
					if (mDataHandler)
						return mDataHandler->handle_notification(this, args);
					return { };
				}
			);

			MaxExternalEventHandler::MessageEventCallback msgCallback = [this](MessageEvent event) {
				if (mProcessing) {
					mEventOutQueue.try_enqueue(event);
					mEventOutClock.delay(0);
				} else {
					handleOutportMessage(event);
				}
			};
#ifdef RNBO_INCLUDE_DESCRIPTION_FILE
			try {
				//we only add a message out if the outport doesn't have a corresponding outlet
				auto outports = patcher_description["outports"];
				if (outports.is_array()) {
					for (auto i: outports) {
						if (outletNames.count(i["tag"]) == 0) {
							mMessageOutlet = std::make_shared<c74::min::outlet<>>(this, "Port Messages Outlet");
							break;
						}
					}
				}
			} catch (std::exception& e) {
				cerr << "exception processing outport json" << c74::min::endl;
			}
#endif
			auto paramCallback = [this](RNBO::ParameterIndex i) {
				auto f = mAttributeLookup.find(i);
				if (f != mAttributeLookup.end()) {
					f->second->touch();
					return;
				}
			};
			MaxExternalEventHandler::PresetTouchedCallback presetTouched = nullptr;
			MaxExternalEventHandler::MidiEventCallback midiEventCallback = [this](MidiEvent midiEvent) {
				if (mProcessing) {
					mEventOutQueue.try_enqueue(midiEvent);
					mEventOutClock.delay(0);
				} else {
					sendMidiEvent(midiEvent);
				}
			};
			RNBO::ScheduleCallback scheduleCallback = nullptr;
			if (mIsInLive) {
 				presetTouched = [this]() { mChangedNotifyDefer.set(); };
				auto x = maxobj();
				scheduleCallback = [x](MillisecondTime time) {
					c74::max::t_atom a;
					c74::max::atom_setfloat(&a, time);
					c74::max::schedulef(x, (c74::max::method)do_trigger, time, nullptr, 1, &a);
				};
			}

			auto transportCallback = [this](RNBO::TransportEvent e) {
				if (mSync)
					mSync->setRunning(e.getState());
			};
			auto tempoCallback = [this](RNBO::TempoEvent e) {
				if (mSync)
					mSync->setTempo(e.getTempo());
			};
			auto beatTimeCallback = [this](RNBO::BeatTimeEvent e) {
				if (mSync)
					mSync->setBeatTime(e.getBeatTime());
			};
			auto timeSigCallback = [this](RNBO::TimeSignatureEvent e) {
				if (mSync)
					mSync->setTimeSignature(e.getNumerator(), e.getDenominator());
			};

			mEventHandler = std::make_unique<MaxExternalEventHandler>(
					this, mRNBOObj,
					scheduleCallback, msgCallback, midiEventCallback,
					transportCallback, tempoCallback, beatTimeCallback, timeSigCallback,
					paramCallback, presetTouched
					);

			mDataHandler = std::make_unique<MaxExternalDataHandler>(this, mRNBOObj);

			for (RNBO::ParameterIndex i = 0; i < mRNBOObj.getNumParameters(); i++) {
				ParameterInfo info;
				mRNBOObj.getParameterInfo(i, &info);

				//only supporting numbers right now
				//don't bind invisible or debug
				if (info.type != ParameterType::ParameterTypeNumber || !info.visible || info.debug)
					continue;
				std::string name = mRNBOObj.getParameterId(i);
				std::shared_ptr<c74::min::attribute_base> a;
				if (info.enumValues == nullptr) {
					 a = std::make_shared<attr_param>(
							this,
							name,
							info.initialValue,
							c74::min::setter {
								[this, i](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
									double in = args[0];
									//cout << "normal set: " << in << c74::min::endl;
									mEventHandler->setParameterValue(i, in);
									return args;
								}
							},
							c74::min::getter {
								[this, i]() -> c74::min::atoms {
									auto v = mEventHandler->getParameterValue(i);
									return c74::min::atoms { v };
								}
							}
					);
				} else {
					//build out list of strings and lookups to convert to from rnbo ParameterValue and max symbols
					std::string initial(info.enumValues[std::min(std::max(0, static_cast<int>(info.initialValue)), info.steps - 1)]);
					//the atoms that min uses to identify the list of enum values
					c74::min::range values;
					//lookup to find a parameter value for an enum symbol (string)
					std::unordered_map<std::string, double> lookup;
					for (int e = 0; e < info.steps; e++) {
						std::string s(info.enumValues[e]);
						values.push_back(s);
						lookup[s] = static_cast<ParameterValue>(e);
					}
					a = std::make_shared<attr_enum_param>(
							this,
							name,
							initial,
							c74::min::setter {
								[this, i, lookup, info](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
									if (args.size() > 0) {
										c74::min::atom in = args[0];
										switch (in.type()) {
											case c74::min::message_type::symbol_argument:
												{
													auto f = lookup.find(in);
													if (f != lookup.end()) {
														mEventHandler->setParameterValue(i, f->second);
													}
												}
												break;
											case c74::min::message_type::int_argument:
											case c74::min::message_type::float_argument:
												{
													double index = in;
													mEventHandler->setParameterValue(i, std::max(std::min(index, static_cast<double>(info.steps - 1)), 0.0));
												}
												break;
											default:
												break;
										}
									}
									return args;
								}
							},
							c74::min::getter {
								[this, i, values]() -> c74::min::atoms {
									auto v = std::min(std::max(static_cast<int>(mEventHandler->getParameterValue(i)), 0), static_cast<int>(values.size() - 1));
									return { values[v] };
								}
							},
							values
					);
				}
				mAttributes.emplace_back(a);
				mAttributeLookup.emplace(i, a);
			}

			mDSPOn = false;

#ifdef RNBO_WRAPPER_INC_TRANSPORT_ATTR
		 mTransportAttr = std::make_unique<attr_transport>(
			this, "transport",
			c74::min::symbol(),
			c74::min::setter {
				[this](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
					if (mSync && args.size() >= 1)
						mSync->updateTransport(args[0]);
					return { };
				}
			},
			c74::min::getter {
				[this]() -> c74::min::atoms {
					return { mSync ? mSync->transportName() : c74::min::k_sym__empty };
				}
			}
		);
#endif
		}

		c74::min::message<c74::min::threadsafe::yes> m_ints {
			this, "int", "MIDI INPUT",
				[this](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
					long v = args[0];
					//if the input is a message input, send inport
					auto it = mMessageInletMap.find(inlet);
					if (it != mMessageInletMap.end()) {
						//add tag and send input
						c74::min::atoms out = {it->second, static_cast<double>(v)};
						handleInportMessage(out);
					} else {
						//do midi
						auto index = inlet - mMidiProcessStart;
						if (index < mMidiProcess.size()) {
							//XXX is maping index to port correct?
							mMidiProcess[index]->process(v, [index, this](ConstByteArray bytes, size_t len) {
								mEventHandler->scheduleEvent(RNBO::MidiEvent(scheduleNow(), index, bytes, len));
							});
						}
					}
					return {};
				}
		};

		c74::min::message<> message {
			this, "message",
			[this](const c74::min::atoms& args, const int _inlet) -> c74::min::atoms {
				handleInportMessage(args);
				return {};
			}
		};

		c74::min::message<> m_float {
			this, "float",
			[this](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
				auto it = mMessageInletMap.find(inlet);
				if (it != mMessageInletMap.end()) {
					//add tag and send input
					c74::min::atoms out = {it->second, args[0]};
					handleInportMessage(out);
				}
				return {};
			}
		};

		c74::min::message<> m_list {
			this, "list",
			[this](const c74::min::atoms& args, const int inlet) -> c74::min::atoms {
				auto it = mMessageInletMap.find(inlet);
				if (it != mMessageInletMap.end()) {
					//add tag and send input
					c74::min::atoms out = {it->second};
					out.insert(out.end(), args.begin(), args.end());
					handleInportMessage(out);
				}
				return {};
			}
		};

		//XXX does the type of value really matter?
		c74::min::attribute<double> mValue {
			this, "value",
			0,
			c74::min::visibility::hide,
			c74::min::getter {
				[this]() -> c74::min::atoms {
					t_dictionary *preset = reinterpret_cast<t_dictionary *>(c74::max::dictionary_new());
					MaxPresetAdapter::getObjectPreset(mRNBOObj, preset, mDSPOn && !mIsInLive);
					return { preset };
				}
			},
			c74::min::setter {
				[this](const c74::min::atoms& args, const int _inlet) -> c74::min::atoms {
					if (args.size() && c74::max::atomisdictionary(const_cast<c74::max::t_atom *>(reinterpret_cast<const c74::max::t_atom *>(&args[0])))) {
						auto preset = args[0].a_w.w_obj; //XXX min doesn't expose dictionary..
						MaxPresetAdapter::setObjectPreset(mRNBOObj, reinterpret_cast<t_dictionary*>(preset));
					}
					return { };
				}
			}
    };

		c74::min::message<> mMaxClassSetup {
			this, "maxclass_setup",
			[this](const c74::min::atoms& args, const int _inlet) -> c74::min::atoms {
				c74::max::t_class * c = args[0];

				c74::max::class_addmethod(c, (c74::max::method) rnbowrapper_parameter_attr_override, "parameter_attr_override", c74::max::A_CANT, 0);

				c74::max::t_atom a;
				//c74::max::atom_setlong(&a, PARAM_TYPE_BLOB);
				//class_parameter_setinfo(c, PARAM_DATA_TYPE_TYPE, 1, &a);

				c74::max::atom_setlong(&a, 1);
				class_parameter_setinfo(c, c74::max::PARAM_DATA_TYPE::PARAM_DATA_TYPE_NOBLOBCACHE, 1, &a);

				return {};
			}
    };

		c74::min::message<> mInstanceSetup {
			this, "setup",
			[this](const c74::min::atoms& args, const int _inlet) -> c74::min::atoms {
				auto x = maxobj();
#ifdef RNBO_WRAPPER_HAS_AUDIO
				mDSPOn = c74::max::sys_getdspobjdspstate(x);
#endif
				object_parameter_init_flags((c74::max::t_object *)x, c74::max::PARAM_TYPE::PARAM_TYPE_BLOB, c74::max::PARAM_FLAGS::PARAM_FLAGS_FORCE_TYPE);
				//make sure that the maxobj is valid when we create sync
				mSync = std::make_unique<Sync>(this);
				mSync->updateCache();
				//c74::max::object_attr_setlong(x, c74::max::gensym("parameter_enable"), 1);
				if (!mDSPOn)
					mProcessClock.delay(0); //start the scheduler
				return {};
			}
    };

		//trampoline, used for calling schedulef
		static void do_trigger(c74::min::minwrap<rnbo_external_wrapper> *x, t_symbol *s, short argc, c74::max::t_atom *argv) {
			if (argc >= 1) {
				MillisecondTime time = c74::max::atom_getfloat(argv);
				x->m_min_object.trigger(time);
			}
		}

		void trigger(MillisecondTime time) {
			mEventHandler->scheduleTriggerEvent(time);
		}

		void sendMidiEvent(MidiEvent midiEvent) {
			if (mMIDIOutlet) {
				int len = midiEvent.getLength();
				ConstByteArray data = midiEvent.getData();
				for (int i = 0; i < len; i++)
					mMIDIOutlet->send(data[i]);
			}
		}

		void send_queued_events() {
			RNBO::EventVariant event;
			while (mEventOutQueue.try_dequeue(event)) {
				switch (event.getType()) {
					case RNBO::Event::Midi:
						sendMidiEvent(event.getMidiEvent());
						break;
					case RNBO::Event::Message:
						handleOutportMessage(event.getMessageEvent());
						break;
					default:
						// we are only interested in Midi and Message events
						break;
				}
			}
		}

		void dsp_on(bool on) {
			LockGuard guard(mDSPStateMutex);
			mDSPOn = on;
			//start non dsp processing
			if (!on) {
				mProcessClock.delay(0);
			}
		}

		void process_nodsp() {
			LockGuard guard(mDSPStateMutex);
			if (mDSPOn || !mDataHandler)
				return;

			MillisecondTime n = now();
			double sr = c74::max::sys_getsr();
			size_t bs = c74::max::sys_getblksize();
			mRNBOObj.prepareToProcess(sr, bs);
			rnbo_process(n, nullptr, 0, nullptr, 0, bs);

			//TODO, schedule multiple blocks at once, delay longer
			double msPerVector = 1000./ sr * (double)bs;
			mProcessClock.delay(msPerVector);
		}

		void rnbo_process(
				MillisecondTime time,
				SampleValue** audioInputs, size_t numInputs,
				SampleValue** audioOutputs, size_t numOutputs,
				size_t sampleFrames) {
			mProcessing.store(true);
			//couple the rnbotime and this object's scheduler's time
			mRNBOObj.setCurrentTime(time);
			if (mSync) {
				mSync->updateTime(time, [this](RNBO::EventVariant event) {
					//TODO should use mEventHandler ?
					mRNBOObj.scheduleEvent(event);
				});
			}
			mRNBOObj.process(audioInputs, numInputs, audioOutputs, numOutputs, sampleFrames, nullptr, nullptr);
			mProcessing.store(false);
		}

#ifdef RNBO_WRAPPER_HAS_AUDIO
		c74::min::message<> dspsetup {
			this, "dspsetup",
			MIN_FUNCTION {
				dsp_on(true);
				auto vs = c74::max::sys_getblksize(); //NOTE cannot trust min's vector_size()
				mScheduleOffset = vs * 1000. / samplerate();
				if (mSync)
					mSync->updateCache();
				mRNBOObj.prepareToProcess(samplerate(), (size_t)vs);
				return {};
			}
		};

		c74::min::message<> mDSPState {
			this, "dspstate",
			[this](const c74::min::atoms& args, const int _inlet) -> c74::min::atoms {
				dsp_on(args[0]);
				return {};
			}
		};

		void operator()(c74::min::audio_bundle input, c74::min::audio_bundle output) {
			//remap signals as max thinks all inlets are signals, but we only care about some
			for (int i = 0; i < mSignalInletsMap.size(); i++) {
				mSignalInlets[i] = input.samples(mSignalInletsMap[i]);
			}

			rnbo_process(now(), &mSignalInlets.front(), mSignalInletsMap.size(), output.samples(), output.channel_count(), output.frame_count());
		}
#endif
	private:
		//sel is optional for messaging errors to user, it lets us use inport message selectors instead of just "message"
		void handleInportMessage(const c74::min::atoms& args, std::string sel = std::string("message")) {
				if (args.size() == 0)
					return;
				//validate input, accumulating nums
				if (args[0].type() != message_type::symbol_argument) {
					mErrorLogger << "message is missing tag" << endl;
					return;
				}

				bool isBang = args.size() == 2 && args[1].type() == message_type::symbol_argument && args[1] == c74::min::k_sym_bang;
				if (!isBang) {
					for (int i = 1; i < args.size(); i++) {
						auto t = args[i].type();
						if (t != message_type::float_argument && t != message_type::int_argument) {
							mErrorLogger << sel << " only allows \"bang\" or numeric values (1 or more)" << endl;
							return;
						}
					}
				}

				//process and send input
				auto tag = TAG(static_cast<std::string>(args[0]).c_str());
				RNBO::MessageTag objectId = TAG("");
				if (isBang) {
					mEventHandler->sendMessage(tag, objectId, scheduleNow());
				} else if (args.size() == 2)
					mEventHandler->sendMessage(tag, static_cast<double>(args[1]), objectId, scheduleNow());
				else {
					auto l = std::make_unique<RNBO::list>();
					for (int i = 1; i < args.size(); i++)
						l->push(args[i]);
					mEventHandler->sendMessage(tag, std::move(l), objectId, scheduleNow());
				}
		}

		void handleOutportMessage(MessageEvent event) {
			//XXX if in process??
			const char* objectId = mRNBOObj.resolveTag(event.getObjectId());
			//skip messages specifically for objects (only for rnbo internal use)
			if (objectId[0] != '\0')
				return;

			//prefix with tag
			const char* tag = mRNBOObj.resolveTag(event.getTag());
			c74::min::atoms atoms = { tag };

			switch (event.getType()) {
				case MessageEvent::Type::Number:
					atoms.push_back(event.getNumValue());
					break;
				case MessageEvent::Type::Bang:
					atoms.push_back(c74::min::k_sym_bang);
					break;
				case MessageEvent::Type::List:
					{
						std::shared_ptr<const RNBO::list> elist = event.getListValue();
						for (size_t i = 0; i < elist->length; i++)
							atoms.push_back(elist->operator[](i));
					}
					break;
				case MessageEvent::Type::Invalid:
				case MessageEvent::Type::Max_Type:
				default:
					return; //TODO warning message?
			}

			if (atoms.size() > 0 && atoms[0].type() == c74::min::message_type::symbol_argument) {
				auto it = mMessageOutletMap.find(atoms[0]);
				if (it != mMessageOutletMap.end() && mOutlets.size() > it->second) {
					//remove tag
					c74::min::atoms out(atoms.begin() + 1, atoms.end());
					mOutlets[it->second]->send(out);
					return;
				}
			}
			if (mMessageOutlet)
				mMessageOutlet->send(atoms);
		}

		std::unique_ptr<MaxExternalDataHandler> mDataHandler;
		std::unique_ptr<MaxExternalEventHandler> mEventHandler;

		//pointer to notify, needs to be dealloced after mDataHandler
		std::unique_ptr<c74::min::message<>> mNotify;

		std::vector<std::unique_ptr<c74::min::inlet<>>> mInlets;
		std::vector<std::shared_ptr<c74::min::outlet<>>> mOutlets;

		//max makes all inlets into signal inlets, but we only care about some of them so we remap before processing
		std::vector<int> mSignalInletsMap; //mSignalInlets[index] = maxinlet[mSignalInletsMap[index]]
		std::vector<double*> mSignalInlets;

		std::unordered_map<unsigned int, t_symbol *> mMessageInletMap;
		std::unordered_map<t_symbol *, size_t> mMessageOutletMap;

		std::shared_ptr<c74::min::outlet<>> mMessageOutlet;

		std::vector<std::shared_ptr<c74::min::attribute_base>> mAttributes;
		std::unordered_map<RNBO::Index, std::shared_ptr<c74::min::attribute_base>> mAttributeLookup;
		std::unique_ptr<attr_transport> mTransportAttr;

		std::vector<std::unique_ptr<c74::min::message<>>> mInportMessages;

		std::vector<std::unique_ptr<RNBO::MidiStreamParser>> mMidiProcess;
		long mMidiProcessStart = 0;

		MillisecondTime mScheduleOffset = 0;

		bool mIsInLive = false;
		bool mDSPOn = false;
		Mutex mDSPStateMutex;

		c74::min::timer<c74::min::timer_options::deliver_on_scheduler> mProcessClock;

		c74::min::queue<> mChangedNotifyDefer { this,
			[this](const c74::min::atoms& _args, const int _inlet) -> c74::min::atoms {
				c74::max::object_notify(this->maxobj(), c74::min::k_sym_modified, nullptr);
				return {};
			}
		};

		MillisecondTime now() {
			return gettime_forobject(maxobj());
		}

		//ripped from rnbo~
		MillisecondTime scheduleNow() {
			MillisecondTime n = now();
#ifdef RNBO_WRAPPER_HAS_AUDIO
			if (!c74::max::systhread_isaudiothread()) {
				// if the event does not arrive in an audio thread (scheduler in audio interrupt)
				// then we want to schedule it for the next buffer
				// this means we introduce latency, but we keep the events times correctly
				// relative to each other
				n += mScheduleOffset;
			}
#endif
			return n;
		}

		c74::min::logger mErrorLogger {
			this, c74::min::logger::type::error
		};

		std::atomic<bool> mProcessing = false;
		c74::min::fifo<RNBO::EventVariant> mEventOutQueue;
		c74::min::timer<c74::min::timer_options::deliver_on_scheduler> mEventOutClock;

		std::shared_ptr<c74::min::outlet<>> mMIDIOutlet;
		std::unique_ptr<Sync> mSync;
		RNBO::CoreObject mRNBOObj;
};

#ifndef RNBO_WRAPPER_MAX_NAME
#error(you must define RNBO_WRAPPER_MAX_NAME)
#else
//expand tokens before calling MIN_EXTERNAL_CUSTOM
#define XMIN_EXTERNAL_CUSTOM(a, b) MIN_EXTERNAL_CUSTOM(a, b)
XMIN_EXTERNAL_CUSTOM(rnbo_external_wrapper, RNBO_WRAPPER_MAX_NAME);
#endif
