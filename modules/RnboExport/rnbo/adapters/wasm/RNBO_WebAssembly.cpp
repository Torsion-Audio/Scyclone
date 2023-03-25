// use the core engine, we do not need thread safety and state restoration
#define RNBO_SIMPLEENGINE
#define RNBO_NO_INT64

#include "RNBO.h"
#include "RNBO.cpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

class WASMParameterEvent : public RNBO::ParameterEvent
{
public:
    WASMParameterEvent(
        RNBO::Index parameterIndex,
        RNBO::MillisecondTime eventTime,
        RNBO::ParameterValue value,
        RNBO::ParameterInterfaceId interfaceId
    ) : RNBO::ParameterEvent(parameterIndex, eventTime, value, interfaceId)
    {}

    uintptr_t getParameterEventSource() const
    {
        return reinterpret_cast<uintptr_t>(getSource());
    }

    void dummySetter(RNBO::number val) {
        // events are immutable, so we introduce a NOOP dummy setter here
    }
};

class WASMMidiEvent : public RNBO::MidiEvent
{
public:
    WASMMidiEvent(
        RNBO::MillisecondTime eventTime,
        int portIndex,
        RNBO::ConstByteArray data,
        int length,
        RNBO::EventTarget* eventTarget = nullptr
    ) : MidiEvent(eventTime, portIndex, data, length, eventTarget)
    {}

    // to avoid having to implement returning an array of uint8 data from a MIDI event
    // i implement returning each byte extra, once we want to support arbitrary length MIDI data
    // we need to change this
    int getB1() const
    {
        if (getLength() > 0) {
            const uint8_t* data = getData();
            return data[0];
        }
        else {
            return 0;
        }
    }

    int getB2() const
    {
        if (getLength() > 1) {
            const uint8_t* data = getData();
            return data[1];
        }
        else {
            return 0;
        }
    }

    int getB3() const
    {
        if (getLength() > 2) {
            const uint8_t* data = getData();
            return data[2];
        }
        else {
            return 0;
        }
    }

    void dummySetter(RNBO::number val) {
        // nothing do do here
    }
};

class WASMMessageEvent : public RNBO::MessageEvent
{
public:
    WASMMessageEvent(const RNBO::MessageEvent& other)
    {
		_tag = other.getTag();
		_objectId = other.getObjectId();
		_eventTime = other.getTime();
		_type = other.getType();

		if (_type == RNBO::MessageEvent::List) {
			std::shared_ptr<const RNBO::list> tmp = other.getListValue();
			for (size_t i = 0; i < tmp->length; i++) {
				_listValueForJS.push_back((*tmp)[i]);
			}
			_typeForJS = 1;
		}
		else if (_type == RNBO::MessageEvent::Number) {
			_numValueForJS = other.getNumValue();
			_typeForJS = 0;
		}
		else if (_type == RNBO::MessageEvent::Bang) {
			_typeForJS = 2;
		}
	}

    void dummySetter(RNBO::number val) {
        // events are immutable, so we introduce a NOOP dummy setter here
    }

	RNBO::number getNumValueForJS() const { return _numValueForJS; }
	std::vector<RNBO::number> getListValueForJS() const { return _listValueForJS; }
	int getTypeForJS() const { return _typeForJS; }

private:
	RNBO::number				_numValueForJS = 0;
	std::vector<RNBO::number> 	_listValueForJS;
	int 						_typeForJS;			// 0 indicates a number, 1 a list, 2 a bang
};

class WASMPresetEvent : public RNBO::PresetEvent
{
public:
    WASMPresetEvent(const RNBO::PresetEvent& event)
	: RNBO::PresetEvent(event)
    {}

    void dummySetter(RNBO::number val) {
        // events are immutable, so we introduce a NOOP dummy setter here
    }

private:
	RNBO::MillisecondTime _time;
};

class WASMEventHandler : public RNBO::EventHandler
{
public:
	void eventsAvailable() {
        drainEvents();
    }
};

struct EventHandlerWrapper : public wrapper<WASMEventHandler> {
    EMSCRIPTEN_WRAPPER(EventHandlerWrapper);
    void handleParameterEvent(const RNBO::ParameterEvent& event) {
        call<void>("handleParameterEvent", WASMParameterEvent(event.getIndex(), event.getTime(), event.getValue(), event.getSource()));
    }
    void handleMidiEvent(const RNBO::MidiEvent& event) {
        const uint8_t* data = event.getData();
        call<void>("handleMidiEvent", WASMMidiEvent(event.getTime(), event.getPortIndex(), event.getData(), event.getLength()));
    }
    void handleMessageEvent(const RNBO::MessageEvent& event) {
        call<void>("handleMessageEvent", WASMMessageEvent(event));
    }
	void handlePresetEvent(const RNBO::PresetEvent& event) {
		if (event.getType() == RNBO::PresetEvent::Touched) {
			call<void>("handlePresetEvent", WASMPresetEvent(event));
		}
	}
};

struct ParameterInfo {
    int type;
	int ioType = RNBO::IOTypeUndefined;
    size_t signalIndex = -1;
};

struct DataType {
    int type = RNBO::DataType::Untyped;

	// for audio buffers
    size_t channels = 0;
	RNBO::number sampleRate = 0;
};

struct DataRefData {
    uintptr_t data = 0;
    size_t sizeInBytes = 0;
};

class CoreObjectWrapper : public RNBO::CoreObject
{
public:
    ~CoreObjectWrapper() {
        RNBO::Index numIns = this->getNumInputChannels() + this->getNumSignalInParameters();
        RNBO::Index numOuts = this->getNumOutputChannels();

        if (_inputs) {
            for (size_t i = 0; i < numIns; i++) {
                delete _inputs[i];
            }
            delete _inputs;
        }
        if (_outputs) {
            for (size_t i = 0; i < numOuts; i++) {
                delete _outputs[i];
            }
            delete _outputs;
        }
    }

    void processWrapper(
        size_t numInputs,
        size_t numOutputs,
        size_t n
    ) {
        this->process(
            _inputs,
            numInputs,
            _outputs,
            numOutputs,
            n
        );
    }

    void scheduleMidiEventWrapper(
        RNBO::MillisecondTime eventTime,
        int portIndex,
        int b1, int b2, int b3
    ) {
        this->scheduleEvent(RNBO::MidiEvent(eventTime, portIndex, b1, b2, b3));
    }

    void scheduleParameterEventWrapper(
        RNBO::Index parameterIndex,
        RNBO::MillisecondTime eventTime,
        RNBO::ParameterValue value,
        uintptr_t source
    ) {
        this->scheduleEvent(RNBO::ParameterEvent(parameterIndex, eventTime, value, reinterpret_cast<RNBO::ParameterInterfaceId>(source)));
    }

	void scheduleTransportEventWrapper(
		RNBO::MillisecondTime eventTime,
		int state
	) {
		this->scheduleEvent(RNBO::TransportEvent(
			eventTime,
			state == 0 ? RNBO::TransportState::STOPPED : RNBO::TransportState::RUNNING
		));
	}

	void scheduleTempoEventWrapper(
		RNBO::MillisecondTime eventTime,
		RNBO::Tempo	tempo
	) {
		this->scheduleEvent(RNBO::TempoEvent(eventTime, tempo));
	}

	void scheduleBeatTimeEventWrapper(
		RNBO::MillisecondTime eventTime,
		RNBO::BeatTime	beatTime
	) {
		this->scheduleEvent(RNBO::BeatTimeEvent(eventTime, beatTime));
	}

	void scheduleTimeSignatureEventWrapper(
		RNBO::MillisecondTime eventTime,
		int	numerator,
		int denominator
	) {
		this->scheduleEvent(RNBO::TimeSignatureEvent(eventTime, numerator, denominator));
	}

    RNBO::ParameterEventInterface* createParameterInterfaceWrapper(
        WASMEventHandler *handler
    ) {
        return this->createParameterInterface(RNBO::ParameterEventInterface::NotThreadSafe, handler).release();
    }

    std::string getParameterNameWrapper(
        RNBO::Index index
    ) {
        return std::string(this->getParameterName(index));
    }

    std::string resolveTagWrapper(
        RNBO::MessageTag tag
    ) {
        return std::string(this->resolveTag(tag));
    }

    ParameterInfo getParameterInfoWrapper(
        RNBO::Index index
    ) {
        RNBO::ParameterInfo info;
        this->getParameterInfo(index, &info);

        ParameterInfo pinfo;
        pinfo.type = info.type;
        pinfo.ioType = info.ioType;
        pinfo.signalIndex = info.signalIndex;

        return pinfo;
    }

    void sendNumMessageWrapper(
        std::string tag,
        std::string objectId,
        RNBO::number value,
        RNBO::MillisecondTime time = RNBO::RNBOTimeNow
    ) {
        this->sendMessage(RNBO::TAG(tag.c_str()), value, RNBO::TAG(objectId.c_str()), time);
    }

    void sendListMessageWrapper(
        std::string tag,
        std::string objectId,
        uintptr_t list,
        size_t length,
        RNBO::MillisecondTime time = RNBO::RNBOTimeNow
    ) {
        std::unique_ptr<RNBO::list> tmp = RNBO::make_unique<RNBO::list>();
        double* _list = reinterpret_cast<double*>(list);

        for (size_t i = 0; i < length; i++) {
            tmp->push(_list[i]);
        }
        this->sendMessage(RNBO::TAG(tag.c_str()), std::move(tmp), RNBO::TAG(objectId.c_str()), time);
    }

    void sendBangMessageWrapper(
        std::string tag,
        std::string objectId,
        RNBO::MillisecondTime time = RNBO::RNBOTimeNow
    ) {
        this->sendMessage(RNBO::TAG(tag.c_str()), RNBO::TAG(objectId.c_str()), time);
    }

    RNBO::Index getNumParametersWrapper() const
    {
        return this->getNumParameters();
    }

	RNBO::ParameterValue getParameterValueWrapper(RNBO::Index index)
    {
        return this->getParameterValue(index);
    }

	void prepareToProcessWrapper(RNBO::number sampleRate, size_t maxBlockSize)
    {
        if (maxBlockSize > _currentBufferSize) {
            RNBO::Index numIns = this->getNumInputChannels() + this->getNumSignalInParameters();
            RNBO::Index numOuts = this->getNumOutputChannels();
            if (_inputs) {
                for (size_t i = 0; i < numIns; i++) {
                    delete _inputs[i];
                }
                delete _inputs;
            }
            if (_outputs) {
                for (size_t i = 0; i < numOuts; i++) {
                    delete _outputs[i];
                }
                delete _outputs;
            }

            _inputs = new RNBO::SampleValue*[numIns];
            for (size_t i = 0; i < numIns; i++) {
                _inputs[i] = new RNBO::SampleValue[maxBlockSize];
            }
            _outputs = new RNBO::SampleValue*[numOuts];
            for (size_t i = 0; i < numOuts; i++) {
                _outputs[i] = new RNBO::SampleValue[maxBlockSize];
            }

            _currentBufferSize = maxBlockSize;
        }

        this->prepareToProcess(sampleRate, maxBlockSize);
    }

    RNBO::Index getNumInputChannelsWrapper() const
    {
        return this->getNumInputChannels();
    }

	RNBO::Index getNumOutputChannelsWrapper() const
    {
        return this->getNumOutputChannels();
    }

    RNBO::Index getNumSignalInParametersWrapper() const
    {
        return this->getNumSignalInParameters();
    }

    RNBO::MillisecondTime getCurrentTimeWrapper()
    {
        return this->getCurrentTime();
    }

	void setCurrentTimeWrapper(RNBO::MillisecondTime time)
    {
        this->setCurrentTime(time);
    }

    uintptr_t getInputChannel(RNBO::Index index) {
        return  (uintptr_t)_inputs[index];
        // return val(typed_memory_view(_currentBufferSize, _inputs[index]));
    }

    uintptr_t getOutputChannel(RNBO::Index index) {
        return (uintptr_t)_outputs[index];
    }

    uintptr_t getNewMemoryBuffer(size_t sizeInBytes)
    {
        char * memoryBuffer = new char[sizeInBytes];
        return (uintptr_t)memoryBuffer;
    }

    void setExternalAudioBuffer(std::string memoryId, uintptr_t data, size_t sizeInBytes, RNBO::Index channels, RNBO::number sampleRate)
    {
        RNBO::DataType info;
        info.type = RNBO::DataType::Float32AudioBuffer;
        info.audioBufferInfo.channels = channels;
        info.audioBufferInfo.samplerate = sampleRate;
        setDataRef(memoryId, data, sizeInBytes, info);
    }

    void setExternalUntypedBuffer(std::string memoryId, uintptr_t data, size_t sizeInBytes)
    {
        RNBO::DataType info;
		info.type = RNBO::DataType::TypedArray;
        setDataRef(memoryId, data, sizeInBytes, info);
    }

    RNBO::DataRefIndex getDataRefIndex(std::string memoryId)
    {
        RNBO::DataRefIndex numRefs = _engine->getPatcher().getNumDataRefs();
        for (RNBO::DataRefIndex i = 0; i < numRefs; i++) {
            auto ref = _engine->getPatcher().getDataRef(i);
            if (memoryId.compare(ref->getName()) == 0) {
                return i;
            }
        }
        return -1;
    }

    DataType getDataRefType(RNBO::DataRefIndex index)
    {
        DataType dtype;
        auto ref = _engine->getPatcher().getDataRef(index);
        auto refType = ref->getType();
        dtype.type = refType.type;
        if (refType.type == RNBO::DataType::Float32AudioBuffer) {
            dtype.channels = refType.audioBufferInfo.channels;
            dtype.sampleRate = refType.audioBufferInfo.samplerate;
        }
        return dtype;
    }

    DataRefData getDataRefData(RNBO::DataRefIndex index)
    {
        DataRefData dData;
        auto ref = _engine->getPatcher().getDataRef(index);
        dData.data = (uintptr_t)ref->getData();
        dData.sizeInBytes = ref->getSizeInBytes();
        return dData;
    }

    void releaseDataRef(RNBO::DataRefIndex index)
    {
        auto ref = _engine->getPatcher().getDataRef(index);
        ref->setData(nullptr, 0);
		_engine->getPatcher().processDataViewUpdate(index, _engine->getCurrentTime());
    }

    uintptr_t getArrayPassingHEAP(size_t size)
    {
        if (size > _currentMaxArraySize) {
            if (_arrayPassingHEAP) {
                delete _arrayPassingHEAP;
            }
            _arrayPassingHEAP = new double[size];
            _currentMaxArraySize = size;
        }
        return (uintptr_t)_arrayPassingHEAP;
    }

	std::string getPresetWrapper() {
		std::shared_ptr<const RNBO::Preset> returnedPreset;
		// we assume that we are using the non threadsafe RNBO_SIMPLEENGINE
		// so the getPreset callback will be called synchronously - if we
		// ever change this, we need to spin wait for the callback
		getPreset(
			[&returnedPreset] (std::shared_ptr<const RNBO::Preset> preset) {
				returnedPreset = preset;
		});
		return RNBO::convertPresetToJSON(*returnedPreset);
	}

	void setPresetWrapper(std::string jsonString) {
		RNBO::UniquePresetPtr preset = RNBO::convertJSONToPreset(jsonString);
		setPreset(std::move(preset));
	}

private:

    void setDataRef(const std::string& memoryId, uintptr_t data, size_t sizeInBytes, RNBO::DataType type) {
        RNBO::DataRefIndex numRefs = _engine->getPatcher().getNumDataRefs();
        for (RNBO::DataRefIndex i = 0; i < numRefs; i++) {
            auto ref = _engine->getPatcher().getDataRef(i);
            if (memoryId.compare(ref->getName()) == 0) {
                ref->setData((char*)data, sizeInBytes, true);
                ref->setType(type);

                _engine->getPatcher().processDataViewUpdate(i, _engine->getCurrentTime());
            }
        }
    }

    RNBO::SampleValue** _inputs = nullptr;
    RNBO::SampleValue** _outputs = nullptr;
    size_t _currentBufferSize = 0;
    double* _arrayPassingHEAP = nullptr;
    size_t _currentMaxArraySize = 0;
};

// class ValueHolder
// {
// public:
// 	ValueHolder() {}

// 	ValueHolder(RNBO::ValueHolder value)
// 	: _value(std::move(value))
// 	{}

// 	RNBO::ValueHolder::Type getType() {
// 		return _value.getType();
// 	}

//     ValueHolder& operator=(ValueHolder other)
// 	{
// 		_value = std::move(other._value);
// 		return *this;
// 	}

// 	ValueHolder(ValueHolder&& other)
// 	: _value(std::move(other._value))
//     {}

// 	ValueHolder(ValueHolder& other)
// 	: _value(std::move(other._value))
//     {}

// private:
// 	RNBO::ValueHolder _value;
// };

EMSCRIPTEN_BINDINGS(rnbo) {
    class_<WASMParameterEvent>("ParameterEvent")
        .constructor<RNBO::Index, RNBO::MillisecondTime, RNBO::ParameterValue, RNBO::ParameterInterfaceId>()
        .property("index", &WASMParameterEvent::getIndex, &WASMParameterEvent::dummySetter)
        .property("value", &WASMParameterEvent::getValue, &WASMParameterEvent::dummySetter)
        .property("time", &WASMParameterEvent::getTime, &WASMParameterEvent::dummySetter)
        .property("source", &WASMParameterEvent::getParameterEventSource, &WASMParameterEvent::dummySetter)
        ;

    class_<WASMMidiEvent>("MidiEvent")
        .constructor<RNBO::MillisecondTime, int, RNBO::ConstByteArray, int, RNBO::EventTarget*>()
        .property("b1", &WASMMidiEvent::getB1, &WASMMidiEvent::dummySetter)
        .property("b2", &WASMMidiEvent::getB2, &WASMMidiEvent::dummySetter)
        .property("b3", &WASMMidiEvent::getB3, &WASMMidiEvent::dummySetter)
        .property("length", &WASMMidiEvent::getLength, &WASMMidiEvent::dummySetter)
        .property("time", &WASMMidiEvent::getTime, &WASMMidiEvent::dummySetter)
        .property("port", &WASMMidiEvent::getPortIndex, &WASMMidiEvent::dummySetter)
        ;

  	register_vector<RNBO::number>("vector<float>");

    class_<WASMMessageEvent>("MessageEvent")
        .constructor<const RNBO::MessageEvent&>()
        .property("tag", &WASMMessageEvent::getTag, &WASMMessageEvent::dummySetter)
        .property("objectId", &WASMMessageEvent::getObjectId, &WASMMessageEvent::dummySetter)
        .property("time", &WASMMessageEvent::getTime, &WASMMessageEvent::dummySetter)
        .property("type", &WASMMessageEvent::getTypeForJS, &WASMMessageEvent::dummySetter)
        .property("numValue", &WASMMessageEvent::getNumValueForJS, &WASMMessageEvent::dummySetter)
        .property("listValue", &WASMMessageEvent::getListValueForJS, &WASMMessageEvent::dummySetter)
        ;

	class_<WASMPresetEvent>("PresetEvent")
		.constructor<const RNBO::PresetEvent&>()
        .property("time", &WASMPresetEvent::getTime, &WASMPresetEvent::dummySetter)
		;

    class_<WASMEventHandler>("EventHandler")
        .allow_subclass<EventHandlerWrapper>("EventHandlerWrapper")
        .function("handleParameterEvent", optional_override([](WASMEventHandler& self, WASMParameterEvent event) {
            return self.WASMEventHandler::handleParameterEvent(event);
        }))
        .function("handleMidiEvent", optional_override([](WASMEventHandler& self, WASMMidiEvent event) {
            return self.WASMEventHandler::handleMidiEvent(event);
        }))
		.function("handleMessageEvent", optional_override([](WASMEventHandler& self, WASMMessageEvent event) {
            return self.WASMEventHandler::handleMessageEvent(event);
        }))
		.function("handlePresetEvent", optional_override([](WASMEventHandler& self, WASMPresetEvent event) {
            return self.WASMEventHandler::handlePresetEvent(event);
        }))
    ;

    value_object<ParameterInfo>("ParameterInfo")
        .field("type", &ParameterInfo::type)
        .field("ioType", &ParameterInfo::ioType)
        .field("signalIndex", &ParameterInfo::signalIndex)
    ;

    value_object<DataType>("DataType")
        .field("type", &DataType::type)
        .field("channels", &DataType::channels)
        .field("sampleRate", &DataType::sampleRate)
    ;

	value_object<DataRefData>("DataRefData")
		.field("data", &DataRefData::data)
		.field("sizeInBytes", &DataRefData::sizeInBytes)
	;

    class_<CoreObjectWrapper>("CoreObject")
        .constructor<>()
        .function("getParameterValue", &CoreObjectWrapper::getParameterValueWrapper)
        .function("prepareToProcess", &CoreObjectWrapper::prepareToProcessWrapper)
        .function("getNumInputChannels", &CoreObjectWrapper::getNumInputChannelsWrapper)
        .function("getNumOutputChannels", &CoreObjectWrapper::getNumOutputChannelsWrapper)
        .function("getNumSignalInParameters", &CoreObjectWrapper::getNumSignalInParametersWrapper)
        .function("process", &CoreObjectWrapper::processWrapper)
        .function("scheduleMidiEvent", &CoreObjectWrapper::scheduleMidiEventWrapper)
        .function("scheduleParameterEvent", &CoreObjectWrapper::scheduleParameterEventWrapper)
        .function("scheduleTransportEvent", &CoreObjectWrapper::scheduleTransportEventWrapper)
        .function("scheduleTempoEvent", &CoreObjectWrapper::scheduleTempoEventWrapper)
        .function("scheduleBeatTimeEvent", &CoreObjectWrapper::scheduleBeatTimeEventWrapper)
        .function("scheduleTimeSignatureEvent", &CoreObjectWrapper::scheduleTimeSignatureEventWrapper)
        .function("createParameterInterface", &CoreObjectWrapper::createParameterInterfaceWrapper, allow_raw_pointers())
        .function("getNumParameters", &CoreObjectWrapper::getNumParametersWrapper)
        .function("getCurrentTime", &CoreObjectWrapper::getCurrentTimeWrapper)
        .function("setCurrentTime", &CoreObjectWrapper::setCurrentTimeWrapper)
        .function("getParameterInfo", &CoreObjectWrapper::getParameterInfoWrapper)
        .function("getParameterName", &CoreObjectWrapper::getParameterNameWrapper)
        .function("sendNumMessage", &CoreObjectWrapper::sendNumMessageWrapper)
        .function("sendListMessage", &CoreObjectWrapper::sendListMessageWrapper)
        .function("sendBangMessage", &CoreObjectWrapper::sendBangMessageWrapper)
		.function("resolveTag", &CoreObjectWrapper::resolveTagWrapper)
        .function("getInputChannel", &CoreObjectWrapper::getInputChannel)
        .function("getOutputChannel", &CoreObjectWrapper::getOutputChannel)
        .function("getNewMemoryBuffer", &CoreObjectWrapper::getNewMemoryBuffer)
        .function("setExternalAudioBuffer", &CoreObjectWrapper::setExternalAudioBuffer)
        .function("setExternalUntypedBuffer", &CoreObjectWrapper::setExternalUntypedBuffer)
        .function("getDataRefIndex", &CoreObjectWrapper::getDataRefIndex)
        .function("getDataRefType", &CoreObjectWrapper::getDataRefType)
        .function("getDataRefData", &CoreObjectWrapper::getDataRefData)
        .function("releaseDataRef", &CoreObjectWrapper::releaseDataRef)
        .function("getArrayPassingHEAP", &CoreObjectWrapper::getArrayPassingHEAP)
        .function("getPreset", &CoreObjectWrapper::getPresetWrapper)
        .function("setPreset", &CoreObjectWrapper::setPresetWrapper)
    ;

    class_<RNBO::ParameterEventInterface>("ParameterEventInterface");

	// register_map<std::string, ValueHolder>("map<std::string, ValueHolder>");
}
