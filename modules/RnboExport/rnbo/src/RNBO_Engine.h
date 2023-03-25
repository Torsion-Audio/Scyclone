//
//  RNBO_Engine.h
//
//  Created by stb on 07/08/15.
//
//

#ifndef _RNBO_Engine_h
#define _RNBO_Engine_h

#include "RNBO.h"

#include "RNBO_EngineCore.h"
#include "RNBO_ParameterEventQueue.h"
#include "RNBO_PatcherState.h"
#include "RNBO_ParameterInterfaceAsync.h"
#include "RNBO_HardwareDevice.h"

#ifdef C74_UNIT_TESTS
#include <JuceHeader.h>
#endif

#include <mutex>
#include <vector>
#include <thread>
#include <map>

namespace RNBO {

	using ExternalDataQueue = EventQueue<ExternalDataEvent, moodycamel::ReaderWriterQueue<ExternalDataEvent>>;
	using ExternalDataRefMap = std::unordered_map<String, std::shared_ptr<ExternalDataRef>, StringHasher >;
	using ExternalDataRefList = Vector<ExternalDataRef*>;

	/**
	 * @private
	 * The Engine is used to drive the RNBO patcher. It holds common code for
	 * feeding data to, driving, and getting data back from the RNBO patcher.
	 */
	class Engine : public EngineCore
	{
	public:

		/**
		 * @brief Construct a new Engine instance to wrap/execute a RNBO
		 * patcher.
		 *
		 * This patcher is generated C++ code taht you can interact with via a
		 * PatcherInterface. All interactions with this code must go through
		 * this Engine interface.
		 */
		Engine();
		~Engine() override;

		PatcherInterface& getPatcher() const override { return *_patcher; }

		/**
		 * @brief Replace the currently running patcher
		 * @return true replacment was successful
		 * @return false otherwise
		 */
		bool setPatcher(UniquePtr<PatcherInterface> patcher) override;

		/**
		 * @brief update the targets for already scheduled events when a new
		 * patcher is set
		 */
		void updatePatcherEventTarget(const EventTarget *oldTarget, PatcherEventTarget *newTarget) override;
		void rescheduleEventTarget(const EventTarget *target) override;

		ParameterEventInterfaceUniquePtr createParameterInterface(ParameterEventInterface::Type type, EventHandler* handler) override;

		bool prepareToProcess(number sampleRate, Index maxBlockSize, bool force = false) override;

		void process(const SampleValue* const* audioInputs, Index numInputs,
					 SampleValue* const* audioOutputs, Index numOutputs,
					 Index sampleFrames,
					 const MidiEventList* midiInput, MidiEventList* midiOutput) override;

		// used by ParameterInterfaceAsync and ParameterInterfaceAsyncImpl
		void queueServiceEvent(const ServiceNotification& event);
		void registerAsyncParameterInterface(ParameterEventInterfaceImplPtr pi);

		/**
		 * @brief Check whether we are within the processing thread
		 *
		 * Mostly used for debugging purposes.
		 *
		 * @return true if called from inside the process() call on the thread
		 * calling process()
		 * @return false otherwise
		 */
		bool insideProcessThread();

		ExternalDataIndex getNumExternalDataRefs() const;
		ExternalDataId getExternalDataId(ExternalDataIndex index) const;
		const ExternalDataInfo getExternalDataInfo(ExternalDataIndex index) const;

		void setExternalData(ExternalDataId memoryId, char *data, size_t sizeInBytes, DataType type, ReleaseCallback callback);
		void releaseExternalData(ExternalDataId memoryId);
		void setExternalDataHandler(ExternalDataHandler* handler);

		void setPresetSync(UniquePresetPtr preset) override;
		ConstPresetPtr getPresetSync() override;

		void beginProcessDataRefs() override;
		void endProcessDataRefs() override;

 private:

		void handleServiceQueue();
		void clearInactiveParameterInterfaces();
		size_t InitialActiveParameterInterfaces() override;

		void updateExternalDataRefs();

		// list of all currently existing parameter interfaces, might contain inactive ones, that
		// will be deleted when a new one is requested
		std::mutex											_registeredParameterInterfacesMutex;
		std::vector<ParameterEventInterfaceImplPtr>			_registeredParameterInterfaces;

		ExternalDataRefMap									_externalDataRefMap;
		ExternalDataRefList									_externalDataRefs;

		ExternalDataQueue									_externalDataQueueIn;
		ExternalDataQueue									_externalDataQueueOut;
		ExternalDataHandler*								_externalDataHandler = nullptr;

		std::mutex											_serviceQueueMutex;
		ServiceNotificationQueue							_serviceQueue;

		// suppress any processing while we are setting a new patcher
		std::atomic<bool>									_inSetPatcher;
		std::atomic<bool>									_inProcess;

		std::thread::id										_processThreadID;

		friend class SetPatcherLocker;
		friend class ProcessLocker;

	};

#ifdef C74_UNIT_TESTS

	class MockProcessBlockImpl
	{
	public:
		MockProcessBlockImpl()
		{
		}

		template <class T>
		void fillCurrentEvents(EventList<EventVariant>& currentEvents, T timeConverter) const
		{
			//		EventList<EventVariant>& currentEvents = engine.getCurrentEvents();
			//		currentEvents.addEvent(MidiEvent(6, 0, 0x90, 0x3C, 0x64));
		}

		void fillAudioInputBuffers(const std::vector<SampleValue*>& audioInputs, size_t sampleFrames) const
		{
		}

		void fillAudioOutputBuffers(const std::vector<SampleValue*>& audioOutputs, size_t sampleFrames) const
		{
		}

		void sendOutgoingMidiEvents(EventList<MidiEvent>& midiEvents,
									MillisecondTime endTime,
									TimeConverter timeConverter)
		{
		}

	private:
		MockProcessBlockImpl& operator=(const MockProcessBlockImpl&) = delete;
	};

	class MockCoreObject : public NullPatcher
	{
	public:
		MockCoreObject()
		: engineInterface(nullptr)
		, test_param1(0)
		, test_param2(1.)
		{}

		void initialize() override
		{
		}

		Index getNumParameters() const override
		{
			return 2;
		}

		ConstCharPointer getParameterName(Index index) const override
		{
			switch (index) {
				case 0:
					return "param1";
				case 1:
					return "param2";
				default:
					return nullptr;
			}
			return nullptr;
		}

		ConstCharPointer getParameterId(Index index) const override
		{
			switch (index) {
				case 0:
					return "/obj1/param1";
				case 1:
					return "/obj2/param2";
				default:
					return nullptr;
			}
			return nullptr;
		}

		void getParameterInfo(Index index, ParameterInfo* info) const override
		{
			switch (index) {
				case 0:
					info->min = 0;
					info->max = 1;
					break;
				case 1:
					info->min = 1;
					info->max = 10;
					break;
				default:
					info->min = 0;
					info->max = 1;
					break;
			}
		}

		ParameterValue getParameterValue(Index index) override
		{
			switch (index) {
				case 0:
					return test_param1;
				case 1:
					return test_param2;
				default:
					return 0;
			}
		}

		void sendParameter(Index index) override {
			this->getEngine()->notifyParameterValueChanged(index, this->getParameterValue(index), false);
		}

		void setParameterValue(Index index, ParameterValue value, MillisecondTime time) override
		{
			switch (index) {
				case 0:
					test_param1 = value;
					this->sendParameter(0);
					break;
				case 1:
					test_param2 = value;
					this->sendParameter(1);
					break;
				default:
					break;
			}
		}

		void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) override
		{

		}

		void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) override
		{

		}

		void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) override
		{

		}

		void process(const SampleValue* const* audioInputs, Index numInputs,
					 SampleValue* const* audioOutputs, Index numOutputs,
					 Index sampleFrames) override
		{

		}

		Index getNumInputChannels() const override { return 0; }
		Index getNumOutputChannels() const override { return 0; }

		Index getNumMidiInputPorts() const override {
			return 0;
		}

		Index getNumMidiOutputPorts() const override {
			return 0;
		}

		void processParameterEvent(Index index, ParameterValue value, MillisecondTime time) override {
			this->setParameterValue(index, value, time);
		}

		void processNormalizedParameterEvent(Index index, ParameterValue value, MillisecondTime time) override {
			this->setParameterValueNormalized(index, value, time);
		}

		void processOutletEvent(EngineLink* sender, Index index, ParameterValue value,  MillisecondTime time) override {
			RNBO_UNUSED(sender);
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
			RNBO_UNUSED(time);
		}

		DataRefIndex getNumDataRefs() const override {
			return 0;
		}

		DataRef* getDataRef(DataRefIndex index) override {
			RNBO_UNUSED(index);
            return nullptr;
		}

		void processDataViewUpdate(DataRefIndex index, MillisecondTime time) override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(time);
		}

		void processOutletAtCurrentTime(EngineLink* sender, Index index, ParameterValue value) override {
			RNBO_UNUSED(sender);
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
		}

		Index getParameterIndexForID(ConstCharPointer /* paramid */) const { return INVALID_INDEX; }

		EngineInterface* engineInterface;
		MillisecondTime currentTime;
		ParameterValue test_param1;
		ParameterValue test_param2;

	};

	PatcherInterface* creaternbomatic()
	{
		return new MockCoreObject();
	}

	extern "C" PatcherFactoryFunctionPtr GetPatcherFactoryFunction(PlatformInterface* platformInterface)
	{
		Platform::set(platformInterface);
		return creaternbomatic;
	}

	class EngineTests : public UnitTest, public EventHandler {

	public:

		EngineTests()
		: UnitTest("Engine")
		{}

		void eventsAvailable() override { }

		virtual void handleParameterEvent(const ParameterEvent& pe) override
		{
			if (pe.getIndex() == 0) {
				if (pe.getValue() != 0.3)
					expect(false, "Wrong value in Parameter 0");
			} else if (pe.getIndex() == 1) {
				if (pe.getValue() != 0.5)
					expect(false, "Wrong value in Parameter 1");
			}
			else {
				expect(false, "Wrong Parameter Index");
			}
		}

		void runTest() override {
			beginTest ("Set Parameter");

			_engine.prepareToProcess(44100, 1024);

			ParameterEventInterfaceUniquePtr pi =
				_engine.createParameterInterface(ParameterEventInterface::SingleProducer, nullptr);

			ParameterEventInterfaceUniquePtr pi2 =
				_engine.createParameterInterface(ParameterEventInterface::SingleProducer, this);

			_engine.process(nullptr, 0, nullptr, 0, 1024, nullptr, nullptr);

			pi->setParameterValue(0, 0.3);
			pi->setParameterValue(1, 0.5);

			_engine.process(nullptr, 0, nullptr, 0, 1024, nullptr, nullptr);
		}

	private:

		RNBO::Engine	_engine;

	};

	static EngineTests engineTests;

#endif // C74_UNIT_TESTS

}  // namespace RNBO


#endif // ifndef _RNBO_Engine_h
