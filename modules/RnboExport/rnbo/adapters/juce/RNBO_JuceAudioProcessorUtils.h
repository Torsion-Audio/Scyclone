//
//  RNBO_JuceAudioProcessorUtils.h
//
//  Created by Rob Sussman on 8/14/15.
//
//

#ifndef _RNBO_JuceAudioProcessorUtils_h
#define _RNBO_JuceAudioProcessorUtils_h

#include "RNBO.h"
#include "RNBO_TimeConverter.h"

#include <juce_audio_formats/juce_audio_formats.h>

namespace RNBO {

	// TODO: chop this file, and related
	namespace JuceAudioProcessorUtils
	{
		void fillCurrentEventsFromMidiBuffer(EventList<EventVariant>& currentEvents,
											 const TimeConverter& timeConverter,
											 juce::MidiBuffer& midiMessages);

		void fillAudioInputBuffers(const std::vector<SampleValue*>& audioInputs,
								   size_t sampleFrames,
								   juce::AudioSampleBuffer& buffer);

		void fillAudioOutputBuffers(const std::vector<SampleValue*>& audioOutputs,
									size_t sampleFrames,
									juce::AudioSampleBuffer& buffer);


		void sendOutgoingMidiEvents(EventList<MidiEvent>& midiEvents,
									MillisecondTime endTime,
									TimeConverter timeConverter,
									juce::MidiBuffer& midiMessages);

		class ProcessBlockImpl
		{
		public:
			ProcessBlockImpl(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages)
			: _buffer(buffer)
			, _midiMessages(midiMessages)
			{

			}

			template <class T>
			void fillCurrentEvents(EventList<EventVariant>& currentEvents, T timeConverter ) const
			{
				JuceAudioProcessorUtils::fillCurrentEventsFromMidiBuffer(currentEvents, timeConverter, _midiMessages);
			}

			void fillAudioInputBuffers(std::vector<SampleValue*>& audioInputs, size_t sampleFrames) const
			{
				JuceAudioProcessorUtils::fillAudioInputBuffers(audioInputs, sampleFrames, _buffer);
			}

			void fillAudioOutputBuffers(std::vector<SampleValue*>& audioOutputs, size_t sampleFrames) const
			{
				JuceAudioProcessorUtils::fillAudioOutputBuffers(audioOutputs, sampleFrames, _buffer);

				RNBO_ASSERT((int) sampleFrames == _buffer.getNumSamples());
			}

			void sendOutgoingMidiEvents(EventList<MidiEvent>& midiEvents,
										MillisecondTime endTime,
										TimeConverter timeConverter)
			{
				JuceAudioProcessorUtils::sendOutgoingMidiEvents(midiEvents, endTime, timeConverter, _midiMessages);
			}

		private:
			ProcessBlockImpl& operator=(const ProcessBlockImpl&) = delete;

			juce::AudioSampleBuffer& _buffer;
			juce::MidiBuffer& _midiMessages;
		};

	}  // namespace JuceAudioProcessorUtils

}  // namespace RNBO

#endif // _RNBO_JuceAudioProcessorUtils_h
