#ifndef _RNBO_TYPES_H_
#define _RNBO_TYPES_H_

#include "RNBO_CompilerMacros.h"
#include "RNBO_Std.h"

/**
 * @file RNBO_Types.h
 * @brief Various type aliases used throughout RNBO
 */


namespace RNBO {

	// let RNBO use 32 bit float instead of double
	// defining this here seems not to be ideal, we might want to have this is RNBO_LocalConfig.h or something similar
	// #define RNBO_USE_FLOAT32

#ifdef RNBO_USE_FLOAT32
	using number = float;
	using SampleValue = float;
#else
	/**
	 * @var typedef double RNBO::number
	 * @brief RNBO treats all numbers as double-precision floating point values
	 */
	using number = double;

	/**
	 * @var typedef double RNBO::SampleValue
	 * @brief RNBO sample values are represented as double-precision floating points
	 */
	using SampleValue = double;
#endif // RNBO_USE_FLOAT32

	/**
	 * @var typedef RNBO::SampleValue* RNBO::signal
	 * @brief a RNBO signal is an array of SampleValues
	 */
	using signal = SampleValue *;

	// avoid denom test for genexpr statements - speeds up computation
#define RNBO_NO_DENORM_TEST

	using generic = number;

	/**
	 * @var typedef number RNBO::ParameterValue
	 * @brief the value of a RNBO parameter
	 */
	using ParameterValue = number;

	/**
	 * @var typedef RNBO:SampleValue RNBO::Sample
	 * @brief an audio sample value
	 */
	using Sample = SampleValue;		// an alias of SampleValue, perhaps we can get rid of this?

	/**
	 * @var typedef RNBO::Sample* RNBO::SampleArray
	 * @brief a modifiable array of samples
	 *
	 * Note: templating allows JS code-generation to specify the array size
	 */
	template <size_t L>
	using SampleArray = Sample*[L];

	/**
	 * @var typedef const RNBO::Sample* RNBO::ConstSampleArray
	 * @brief a read-only array of samples
	 *
	 * Note: templating allows JS code-generation to specify the array size
	 */
	template <size_t L>
	using ConstSampleArray = const Sample*[L];

	/**
	 * @var typedef RNBO::number RNBO::MillisecondTime
	 * @brief working with time is usually done in milliseconds
	 */
	using MillisecondTime = number;

	/**
	 * @var typedef RNBO::number RNBO::Tempo
	 * @brief tempo (BPM) is represented as a floating point
	 */
	using Tempo = number;

	/**
	 * @var typedef RNBO::number RNBO::BeatTime
	 * @brief time in terms of quarter notes
	 *
	 * Beat time is commonly used in conjunction with Ableton Link.
	 */
	using BeatTime = number;

	enum TransportState
	{
		STOPPED,
		RUNNING
	};

	/**
	 * @var typedef size_t RNBO::Index
	 * @brief an index into an array
	 */
	using Index = size_t;

	/**
	 * @var typedef intptr_t RNBO::Int
	 * @brief the integer type for RNBO
	 */
	using Int = intptr_t;

#ifdef RNBO_USE_FLOAT32
	using UInt = uint32_t;
#else
	using UInt = uint64_t;
#endif

	const Index INVALID_INDEX = SIZE_MAX;

	/**
	 * @var typedef RNBO::Index RNBO::ParameterIndex
	 * @brief a parameter identifier
	 */
	using ParameterIndex = Index;

	/**
	 * @var typedef RNBO::Index RNBO::MessageIndex
	 * @brief a message identifier
	 */
	using MessageIndex = Index;

	/**
	 * @var typedef RNBO::Index RNBO::OutletIndex
	 * @brief an outlet identifier
	 */
	using OutletIndex = Index;

	/**
	 * @var typedef RNBO::Index RNBO::SignalIndex
	 * @brief an signal identifier
	 */
	using SignalIndex = Index;

	/**
	 * @var typedef RNBO::Int RNBO::SampleIndex
	 * @brief a sample array index
	 */
	using SampleIndex = Int;

	/**
	 * @var typedef RNBO::Int RNBO::SampleOffset
	 * @brief used to represent an index offset from some origin
	 */
	using SampleOffset = Int;

	/**
	 * @var typedef RNBO::Int RNBO::ClockId
	 * @brief a clock identifier
	 */
	using ClockId = Int;

	/**
	 * @var typedef const void* RNBO::ParameterInterfaceId
	 * @brief an identifier for a ParameterInterface
	 */
	using ParameterInterfaceId = const void*;

	/**
	 * @var typedef RNBO::Int RNBO::DataRefIndex
	 * @brief an index into an external data reference
	 *
	 * In order to encode additional information (such as the absence of a DataRef), negative values are allowed.
	 */
	using DataRefIndex = Int;

	/**
	 * @var typedef RNBO::Int RNBO::ProbingIndex
	 * @brief an index into the array of signal probes
	 *
	 * Internal use.
	 *
	 * In order to encode additional information, negative values are allowed.
	 */
	using ProbingIndex = Int;

	/**
	 * @var typedef const char * RNBO::ConstCharPointer
	 * @brief const pointer to char
	 */
	using ConstCharPointer = const char *;

	/**
	 * @var typedef const uint8_t RNBO::ConstByteArray
	 * @brief an alias to aid type annotation
	 */
	using ConstByteArray = const uint8_t *;

	//.*BinOpInt have to be 32 bit for javascript calculation parity
	/**
	 * @var typedef int32_t RNBO::BinOpInt
	 * @brief a signed integer for bitwise operations
	 */
	using BinOpInt = int32_t;

	/**
	 * @var typedef uint32_t RNBO::UBinOpInt
	 * @brief an unsigned integer for bitwise operations
	 */
	using UBinOpInt = uint32_t;

	static const MillisecondTime RNBOTimeNow = 0;

	/** Valid types of parameters */
	enum ParameterType
	{
		ParameterTypeNumber = 0,  ///< a number parameter (e.g. from param)
		ParameterTypeBang,        ///< a bang
		ParameterTypeList,        ///< unimplemented, placeholder
		ParameterTypeSignal,      ///< a signal parameter (e.g. from param~)
		ParameterTypeCount
	};

	/** Valid types of I/O */
	enum IOType
	{
		IOTypeInput,      ///< input
		IOTypeOutput,     ///< output
		IOTypeUndefined   ///< undefined
	};

	struct ParameterInfo
	{
		ParameterType type;
		IOType ioType = IOTypeUndefined;
		ParameterValue min = 0.;
		ParameterValue max = 1.;
		ParameterValue initialValue = 0.;
		ParameterValue exponent = 1.;
		int steps = 0; // 0 = continuous, 1 = toggle, 2+ indicates steps in the normalized representation
		bool debug = false;
		bool saveable = false;
		bool transmittable = false;
		bool initialized = false;
		bool visible = false;
		const char **enumValues = nullptr;
		const char *displayName = "";
		const char *unit = "";
		SignalIndex signalIndex = INVALID_INDEX;
	};

	enum LogLevel
	{
		Info,
		Warning,
		Error
	};

	/** Get the address of an object */
	template<typename T> T* addressOf(T& object) { return &object; }

	/**
	 * @typedef uint32_t RNBO::MessageTag
	 * @brief a unique identifier for a tag
	 *
	 * Generated via hashing the message name
	 */
	using MessageTag = uint32_t;

	// a simple SDBM Hash function http://www.partow.net/programming/hashfunctions/#SDBMHashFunction
	/**
	 * @brief Generate a MessageTag from a message name via SDBM hashing
	 *
	 * @param str the message name
	 * @param hash an initial hash value
	 * @return a MessageTag
	 *
	 * @code{.cpp}
	 * TAG("test")  // = 1195757874
	 * @endcode
	 */
	constexpr MessageTag TAG(const char* str, MessageTag hash = 0) {
		return (!(*str))
		? hash
		: TAG(str + 1, static_cast<MessageTag>(*str) + (hash << 6) + (hash << 16) - hash);
	}

	constexpr MessageTag ID(const char* str) { return TAG(str); }

	static_assert(TAG("test") == 1195757874, "");	// testing if the const expr can actually be evaluted at compile time
	static_assert(TAG("") == 0, "");
	static_assert(ID("test") == 1195757874, "");
	static_assert(ID("") == 0, "");

	/**
	 * @var typedef const char* RNBO::MessageTagInfo
	 * @brief the human-readable name of an inport or outport
	 */
	using MessageTagInfo = const char*;

	/** Valid message port types */
	enum MessagePortType
	{
		Inport,      ///< an inport
		Outport,     ///< an outport
		Undefined    ///< undefined (e.g. for null messages)
	};

	struct MessageInfo
	{
		MessageTagInfo	tag;
		MessagePortType type;
	};

	static const MessageInfo NullMessageInfo = { "", MessagePortType::Undefined };

// member funs macro as recommended by isocpp.org FAQ
#define CALL_MEMBER_FN(ptrToObject,ptrToMember)  ((*ptrToObject).*(ptrToMember))

// quoting magic from http://stackoverflow.com/questions/6671698/adding-quotes-to-argument-in-c-preprocessor
/**
 * @def RNBO_Q(x)
 * Adds quotes within preprocessor macros
 */
#define RNBO_Q(x) #x
#define RNBO_QUOTE(x) RNBO_Q(x)

} // namespace RNBO

#endif // #ifndef _RNBO_TYPES_H_
