//
//  RNBO_TimeConverter.h
//
//  Created by stb on 07/08/15.
//
//

#ifndef _RNBO_TimeConverter_h
#define _RNBO_TimeConverter_h

namespace RNBO {

	/**
	 * @brief Utility class to convert between sample indices and milliseconds
	 */
	class TimeConverter
	{
	public:
		/**
		 * @brief Construct a TimeConverter utility object
		 *
		 * @param sr the samplerate
		 * @param currentTime the current time in milliseconds
		 */
		TimeConverter(number sr, MillisecondTime currentTime)
		: _sampleRate(sr)
		, _currentTime(currentTime)
		, _millisecondsPerSample(1000. / static_cast<MillisecondTime>(_sampleRate))
		, _samplesPerMillisecond(static_cast<MillisecondTime>(_sampleRate) / 1000.)
		{

		}

		/**
		 * @brief Convert a sample value to milliseconds
		 * 
		 * @param samples the sample value to convert
		 * @return the sample value in milliseconds
		 * 
		 * @code{.cpp}
		 * // Convert 400 samples at 44100 kHz sampling rate to a millisecond time
		 * TimeConverter timeConverter(44100, 0);
		 * MillisecondTime t = timeConverter.convertSampleIndexToMilliseconds(400);
		 * @endcode
		 */
		inline MillisecondTime convertSampleIndexToMilliseconds(long samples) const
		{
			return static_cast<MillisecondTime>(static_cast<MillisecondTime>(samples) * _millisecondsPerSample);
		}

		/**
		 * @brief Convert millisecond time to a sample value
		 *
		 * @param milliseconds the millisecond time to convert
		 * @return the millisecond time in samples
		 * @code{.cpp}
		 * // Convert 400 milliseconds to a sample value at 44100 kHz sampling rate
		 * TimeConverter timeConverter(44100, 0);
		 * SampleIndex s = timeConverter.convertMillisecondsToSamples(400);
		 * @endcode
		 */
		inline SampleIndex convertMillisecondsToSamples(MillisecondTime milliseconds) const
		{
			return SampleIndex(milliseconds * _samplesPerMillisecond + .5);
		}

		// "Offset" conversion routines:
		//  - convert to/from number of samples relative to the "currentTime" member of the context

		/**
		 * @brief Convert from a sample offset to millisecond time
		 *
		 * @param sampleOffset the sample offset to convert
		 * @return the millisecond time offset relative to the converter's `currentTime` member
		 *
		 * @code{.cpp}
		 * // Get the millisecond time offset of 300 samples after 100 milliseconds
		 * // at a 44.1 kHz sampling rate
		 * TimeConverter timeConverter(44100, 100);
		 * MillisecondTime t = timeConverter.convertSampleOffsetToMilliseconds(300);
		 * @endcode
		 */
		inline MillisecondTime convertSampleOffsetToMilliseconds(SampleOffset sampleOffset) const
		{
			MillisecondTime msOffset = convertSampleIndexToMilliseconds(static_cast<long>(sampleOffset));
			return _currentTime + msOffset;
		}

		/**
		 * @brief Convert from time in milliseconds to a sample offset
		 *
		 * Calculates the difference between the input millisecond time argument
		 * and the TimeConverter's current time and converts to a sample value.
		 *
		 * @param milliseconds the millisecond time to convert
		 * @return the sample offset relative to the converter's `currentTime` member
		 *
		 * @code{.cpp}
		 * // Get the sample offset of 300 milliseconds after 100 milliseconds
		 * // at a 44.1 kHz sampling rate
		 * TimeConverter timeConverter(44100, 100);
		 * SampleOffset s = timeConverter.convertMillisecondsToSampleOffset(300);
		 * @endcode
		 */
		inline SampleOffset convertMillisecondsToSampleOffset(MillisecondTime milliseconds) const
		{
			MillisecondTime msOffset = milliseconds - _currentTime;
			SampleIndex samples = convertMillisecondsToSamples(msOffset);
			return SampleOffset(samples);
		}
	private:
		number _sampleRate;
		MillisecondTime _currentTime;
		MillisecondTime _millisecondsPerSample;
		MillisecondTime _samplesPerMillisecond;
	};


}  // namespace RNBO


#endif // ifndef _RNBO_TimeConverter_h

