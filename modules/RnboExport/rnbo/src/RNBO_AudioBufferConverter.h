//
//  RNBO_AudioBufferConverter.h
//
//  Created by Rob Sussman on 1/7/16.
//
//

#ifndef _RNBO_AudioBufferConverter_h
#define _RNBO_AudioBufferConverter_h

#include "RNBO_Types.h"
#include "RNBO_Vector.h"

namespace RNBO {

	//typedefs to simplify conversion, Conv is the other bit depth
#ifdef RNBO_USE_FLOAT32
	using SampleValueConv = double;
#else
	using SampleValueConv = float;
#endif

	/**
	 * @private
	 * Container class for audio data to be converted
	 */
	class AudioBufferConverterData {
	public:
		~AudioBufferConverterData() {
			deleteBuffers();
		}

		void setup(size_t numChannels, size_t sampleFrames) {
			//TODO use platform interface
			if (numChannels != _numChannels || sampleFrames != _sampleFrames) {
				deleteBuffers();
				_conversionBuffer  = new SampleValue*[numChannels];
				for (size_t i = 0; i < numChannels; i++) {
					_conversionBuffer[i] = new SampleValue[sampleFrames];
				}

				_numChannels = numChannels;
				_sampleFrames = sampleFrames;
			}
		}

		SampleValue** getConversionBuffer() {
			return _conversionBuffer;
		}

		size_t getNumChannels() {
			return _numChannels;
		}

		size_t getSampleFrames() {
			return _sampleFrames;
		}

	private:

		void deleteBuffers() {
			if (_conversionBuffer) {
				for (size_t i = 0; i < _numChannels; i++) {
					delete _conversionBuffer[i];
				}
				delete _conversionBuffer;
				_conversionBuffer = nullptr;
			}
		}

		size_t _numChannels = 0;
		size_t _sampleFrames = 0;
		SampleValue** _conversionBuffer = nullptr;
	};

	/**
	 * @private
	 * Convert input audio data to internal audio buffer data.
	 */
	class AudioBufferConverterBase {
	public:
		AudioBufferConverterBase(AudioBufferConverterData* data)
		: _data(data)
		{}

		const SampleValue* const * getReadBuffers() {
			return const_cast<const SampleValue* const *>(_audioBufferInternalPointers);
		}

		SampleValue* const * getWriteBuffers() {
			return const_cast<SampleValue* const *>(_audioBufferInternalPointers);
		}

		template <typename U>
		static void clearAudioBuffers(U audioBuffers, size_t numChannels, size_t sampleFrames, size_t startChannel = 0) {
			for (size_t chan = startChannel; chan < numChannels; chan++) {
				auto pDest = audioBuffers[chan];

				size_t n = sampleFrames;
				while (n--) {
					*pDest++ = 0;
				}
			}
		}

	protected:

		template <typename U, typename V>
		void copyBuffer(U src, V* const* dst, size_t numChannels, size_t sampleFrames)
		{
			for (size_t i = 0; i < numChannels; i++) {
				for (size_t j = 0; j < sampleFrames; j ++) {
					dst[i][j] = static_cast<V>(src[i][j]);
				}
			}
		}

		template <typename U, typename V>
		void copyFromInterleaved(U src, V dst, size_t numChannels, size_t sampleFrames)
		{
			for (size_t i = 0; i < numChannels; i++) {
				for (size_t j = 0; j < sampleFrames; j ++) {
					dst[i][j] = src[j * numChannels + i];
				}
			}
		}

		template <typename U, typename V>
		void copyToInterleaved(U src, V * const dst, size_t numChannels, size_t sampleFrames)
		{
			for (size_t i = 0; i < numChannels; i++) {
				for (size_t j = 0; j < sampleFrames; j ++) {
					dst[j * numChannels + i] = static_cast<V>(src[i][j]);
				}
			}
		}

		SampleValue** 				_audioBufferInternalPointers = nullptr;
		AudioBufferConverterData*	_data;
	};

	/**
	 * @private
	 * only use the specializations
	 */
	template <typename T>
	class AudioInBufferConverter : public AudioBufferConverterBase {
	private:
		AudioInBufferConverter();
	};

	// these converters only make sense when SampleValue != converted value
	/**
	 * @private
	 * Convert to double pointers
	 */
	template <>
	class AudioInBufferConverter<const SampleValueConv* const*> : public AudioBufferConverterBase {
	public:
		AudioInBufferConverter(const SampleValueConv* const* audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		{
			_data->setup(numChannels, sampleFrames);
			_audioBufferInternalPointers = _data->getConversionBuffer();
			copyBuffer(audioBuffers, _audioBufferInternalPointers, numChannels, sampleFrames);
		}
	};


	//interleaved
	template <>
	class AudioInBufferConverter<SampleValue const*> : public AudioBufferConverterBase {
	public:
		AudioInBufferConverter(SampleValue const* audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		{
			_data->setup(numChannels, sampleFrames);
			_audioBufferInternalPointers = _data->getConversionBuffer();
			copyFromInterleaved(audioBuffers, _audioBufferInternalPointers, numChannels, sampleFrames);
		}
	};

	template <>
	class AudioInBufferConverter<SampleValueConv const*> : public AudioBufferConverterBase {
	public:
		AudioInBufferConverter(SampleValueConv const* audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		{
			_data->setup(numChannels, sampleFrames);
			_audioBufferInternalPointers = _data->getConversionBuffer();
			copyFromInterleaved(audioBuffers, _audioBufferInternalPointers, numChannels, sampleFrames);
		}
	};

	//diff const versions
	template <>
	class AudioInBufferConverter<SampleValue* const*> : public AudioBufferConverterBase {
	public:
		AudioInBufferConverter(SampleValue* const* audioBuffers, size_t /* numChannels */, size_t /* sampleFrames */, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		{
			//simply forward existing pointer
			_audioBufferInternalPointers = const_cast<SampleValue**>(audioBuffers);
		}
	};

	template <>
	class AudioInBufferConverter<SampleValue**> : public AudioInBufferConverter<SampleValue *const*> {
	public:
		AudioInBufferConverter(SampleValue** audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioInBufferConverter<SampleValue* const*>(audioBuffers, numChannels, sampleFrames, data)
		{
		}
	};

	template <>
	class AudioInBufferConverter<SampleValueConv* const*> : public AudioInBufferConverter<const SampleValueConv* const*> {
	public:
		AudioInBufferConverter(SampleValueConv* const* audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioInBufferConverter<const SampleValueConv * const*>(audioBuffers, numChannels, sampleFrames, data)
		{ }
	};

	template <>
	class AudioInBufferConverter<SampleValueConv**> : public AudioInBufferConverter<const SampleValueConv* const*> {
	public:
		AudioInBufferConverter(SampleValueConv** audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioInBufferConverter<const SampleValueConv * const*>(audioBuffers, numChannels, sampleFrames, data)
		{ }
	};

	template <>
	class AudioInBufferConverter<SampleValueConv *> : public AudioInBufferConverter<SampleValueConv const *> {
	public:
		AudioInBufferConverter(SampleValueConv const* audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
			: AudioInBufferConverter<SampleValueConv const *>(audioBuffers, numChannels, sampleFrames, data)
		{ }
	};

	/**
	 * @private
	 * only use the specializations
	 */
	template <typename T>
	class AudioOutBufferConverter : public AudioBufferConverterBase {
	private:
		// only use the specializations
		AudioOutBufferConverter();
	};


	template <>
	class AudioOutBufferConverter<SampleValueConv* const*> : public AudioBufferConverterBase {
	public:
		AudioOutBufferConverter(SampleValueConv* const* audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		, _audioBuffers(audioBuffers)
		{
			_data->setup(numChannels, sampleFrames);
			_audioBufferInternalPointers = _data->getConversionBuffer();
			_audioBuffers = audioBuffers;
		}

		~AudioOutBufferConverter() {
			copyBuffer(_audioBufferInternalPointers, _audioBuffers, _data->getNumChannels(), _data->getSampleFrames());
		}

	private:
		SampleValueConv* const* _audioBuffers;
	};

	//interleaved
	template <>
	class AudioOutBufferConverter<SampleValue * const> : public AudioBufferConverterBase {
	public:
		AudioOutBufferConverter(SampleValue * const audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		, _audioBuffers(audioBuffers)
		{
			_data->setup(numChannels, sampleFrames);
			_audioBufferInternalPointers = _data->getConversionBuffer();
		}

		~AudioOutBufferConverter() {
			copyToInterleaved(_audioBufferInternalPointers, _audioBuffers, _data->getNumChannels(), _data->getSampleFrames());
		}

	private:
		SampleValue * const _audioBuffers;
	};

	template <>
	class AudioOutBufferConverter<SampleValueConv * const> : public AudioBufferConverterBase {
	public:
		AudioOutBufferConverter(SampleValueConv * const audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		, _audioBuffers(audioBuffers)
		{
			_data->setup(numChannels, sampleFrames);
			_audioBufferInternalPointers = _data->getConversionBuffer();
		}

		~AudioOutBufferConverter() {
			copyToInterleaved(_audioBufferInternalPointers, _audioBuffers, _data->getNumChannels(), _data->getSampleFrames());
		}

	private:
		SampleValueConv * const _audioBuffers;
	};

	//diff const versions
	template <>
	class AudioOutBufferConverter<SampleValue**> : public AudioBufferConverterBase {
	public:
		AudioOutBufferConverter(SampleValue** audioBuffers, size_t /* numChannels */, size_t /* sampleFrames */, AudioBufferConverterData* data)
		: AudioBufferConverterBase(data)
		{
			//simply forward the pointer
			_audioBufferInternalPointers = const_cast<SampleValue**>(audioBuffers);
		}

	};

	template <>
	class AudioOutBufferConverter<SampleValueConv**> : public AudioOutBufferConverter<SampleValueConv* const*> {
	public:
		AudioOutBufferConverter(SampleValueConv** audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioOutBufferConverter<SampleValueConv* const*>(audioBuffers, numChannels, sampleFrames, data)
		{ }
	};

	template <>
	class AudioOutBufferConverter<SampleValueConv *> : public AudioOutBufferConverter<SampleValueConv * const> {
	public:
		AudioOutBufferConverter(SampleValueConv * audioBuffers, size_t numChannels, size_t sampleFrames, AudioBufferConverterData* data)
		: AudioOutBufferConverter<SampleValueConv * const>(audioBuffers, numChannels, sampleFrames, data)
		{ }
	};

} // namespace RNBO

#endif // #ifndef _RNBO_AudioBufferConverter_h

