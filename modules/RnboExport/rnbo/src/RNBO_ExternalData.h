#ifndef _RNBO_ExternalData_h_
#define _RNBO_ExternalData_h_

#include <memory>
#include <string>
#include <functional>

#include "RNBO_Types.h"
#include "RNBO_String.h"
#include "RNBO_DataRef.h"

/**
 * @file RNBO_ExternalData.h
 * @brief External data references are handles to arbitrary data buffers in RNBO
 */

namespace RNBO {

    /**
     * @var typedef const char* RNBO::ExternalDataId
     * @brief The name of an external data reference
     */
	using ExternalDataId	= const char*;

    /**
     * @var typedef int RNBO::ExternalDataIndex
     * @brief The index of an external data reference
     */
	using ExternalDataIndex = int;

    /**
     * @var typedef std::function<void(ExternalDataId, char*)> RNBO::ReleaseCallback
     * @brief A function to call when an external data reference is released
     */
	using ReleaseCallback = std::function<void(ExternalDataId, char*)>;

	/**
	 * @brief A DataBuffer that has no type
	 */
	struct UntypedDataBuffer : public DataType {
		UntypedDataBuffer() {
			type = DataType::Untyped;
		}
	};

	static_assert(sizeof(UntypedDataBuffer) == sizeof(DataType), "Do not add any members to the derived class.");

	/**
	 * @brief A data buffer for 32-bit floating point audio
	 */
	struct Float32AudioBuffer : public DataType
	{
		Float32AudioBuffer(Index channels, number samplerate) {
			type = DataType::Float32AudioBuffer;
			audioBufferInfo.channels = channels;
			audioBufferInfo.samplerate = samplerate;
		}
	};

	static_assert(sizeof(Float32AudioBuffer) == sizeof(DataType), "Do not add any members to the derived class.");

	/**
	 * @brief A data buffer for 64-bit floating point audio
	 */
	struct Float64AudioBuffer : public DataType
	{
		Float64AudioBuffer(Index channels, number samplerate) {
			type = DataType::Float64AudioBuffer;
			audioBufferInfo.channels = channels;
			audioBufferInfo.samplerate = samplerate;
		}
	};

	static_assert(sizeof(Float64AudioBuffer) == sizeof(DataType), "Do not add any members to the derived class.");

	/**
	 * @brief A handle to an external data reference
	 */
	class ExternalDataRef {
	public:
		ExternalDataRef(DataRefIndex index, DataRef *ref)
		: _index(index)
		, _dataRef(ref)
		, _name(ref->getName())
		{
			if (ref->getFile() == nullptr) _file.clear();
			else _file = ref->getFile();
			if (ref->getTag() == nullptr) _tag.clear();
			else _tag = ref->getTag();
			_data = _dataRef->getData();
		}

		const char* getMemoryId() const {
			return _name.c_str();
		}

		const char* getFile() const {
			return _file.c_str();
		}

		const char *getTag() const {
			return _tag.c_str();
		}

		DataRefIndex getInternalIndex() {
			return _index;
		}

		char* getData() const {
			if (_dataRef) return _dataRef->getData();
			else return _data;
		}

		bool isValid() const {
			return _dataRef != nullptr;
		}

		void invalidate() {
			_dataRef = nullptr;
		}

		void revalidate(DataRefIndex index, DataRef *ref) {
			RNBO_ASSERT(!isValid());
			Platform::get()->assertTrue(ref, "ref must be non null");

			if (_callback && _data && _data != ref->getData()) {
				_callback(getMemoryId(), _data);
			}

			_index = index;
			_dataRef = ref;
			if (ref->getFile() == nullptr) _file.clear();
			else _file = ref->getFile();
			if (ref->getTag() == nullptr) _tag.clear();
			else _tag = ref->getTag();
		}

		void updateDataRef(char* data, size_t sizeInBytes) {
			Platform::get()->assertTrue(_dataRef, "_dataRef must be non null");

			if (_callback && _dataRef->getData() && data != _dataRef->getData()) {
				_callback(getMemoryId(), _dataRef->getData());
			}

			_dataRef->setData(data, sizeInBytes);
			_data = data;
		}

		void updateDataRef(char* data, size_t sizeInBytes, DataType type) {
			Platform::get()->assertTrue(_dataRef, "_dataRef must be non null");

			updateDataRef(data, sizeInBytes);
			_dataRef->setType(type);
		}

		void updateDataRef(char* data, size_t sizeInBytes, ReleaseCallback callback) {
			Platform::get()->assertTrue(_dataRef, "_dataRef must be non null");

			updateDataRef(data, sizeInBytes);
			_callback = callback;
		}

		void updateDataRef(char* data, size_t sizeInBytes, DataType type, ReleaseCallback callback) {
			updateDataRef(data, sizeInBytes, type);
			_callback = callback;
		}

		void setTouched(bool value) {
			if (_dataRef) _dataRef->setTouched(value);
		}

		bool getTouched() const {
			if (_dataRef) return _dataRef->getTouched();
			else return false;
		}

		const DataType getType() const {
			if (_dataRef) return _dataRef->getType();
			else return UntypedDataBuffer();
		}

		size_t getSizeInBytes() const {
			if (_dataRef) return _dataRef->getSizeInBytes();
			else return 0;
		}

		ReleaseCallback getCallback() {
			return _callback;
		}

	private:
		DataRefIndex	_index = -1;
		DataRef* 		_dataRef = nullptr;
		std::string 	_name;				// make a copy of the name to avoid accessing
											// undefined const chars when the old patcher goes away
		std::string 	_file;
		std::string		_tag;
		char*			_data = nullptr;	// safe away the data pointer so we can hand it out for freeing
		ReleaseCallback	_callback = nullptr;
	};

	/**
	 * @private
	 */
	class ExternalDataEvent
	{
	public:

		enum class EventAction {
			Undefined,
			SetExternalData,
			ReleaseExternalData
		};

		ExternalDataEvent()
		: _data(nullptr)
		, _sizeInBytes(0)
		, _action(EventAction::Undefined)
		, _callback(nullptr)
		{}

		ExternalDataEvent(const ExternalDataEvent& other) = default;
		ExternalDataEvent& operator = (const ExternalDataEvent& other) = default;

		ExternalDataEvent(String memoryId, char* data, size_t sizeInBytes, DataType type, EventAction action, ReleaseCallback callback)
		: _memoryId(std::make_shared<String>(memoryId))
		, _data(data)
		, _sizeInBytes(sizeInBytes)
		, _type(type)
		, _action(action)
		, _callback(callback)
		{
		}

		const String& getMemoryId() const { return *_memoryId; }
		char *getData() const { return _data; }
		size_t getSizeInBytes() const { return _sizeInBytes; }
		DataType getType() const { return _type; }
		EventAction getAction() const { return _action; }
		ReleaseCallback getCallback() const { return _callback; }

	private:
		std::shared_ptr<String>	_memoryId;
		char*					_data;
		size_t					_sizeInBytes;
		DataType				_type;

		EventAction				_action;
		ReleaseCallback 		_callback;
	};


	using UpdateRefCallback = const std::function<void(DataRefIndex, char*, size_t, DataType)>;
	using ReleaseRefCallback = const std::function<void(DataRefIndex)>;
	using ConstRefList = const ExternalDataRef* const*;

    /**
     * A handler class whose methods can be implemented and associated with an external data reference.
     */
	class ExternalDataHandler
	{
	public:
		virtual ~ExternalDataHandler() {}

		virtual void processBeginCallback(DataRefIndex numRefs, ConstRefList refList, UpdateRefCallback updateDataRef, ReleaseRefCallback releaseDataRef) = 0;
		virtual void processEndCallback(DataRefIndex numRefs, ConstRefList refList) = 0;
	};

	/**
	 * @brief Metadata about an external data reference
	 */
	struct ExternalDataInfo {
		DataType::Type	type;			///< The external data reference type
		const char* 	file;			///< The associated filename
		const char*		tag;			///< Arbitrary tag associated with the external data (usually buffer~ or data)
	};

	constexpr const char *InValidExternalDataId = "";
}

#endif  // _RNBO_ExternalData_h_
