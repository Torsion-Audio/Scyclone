//
//  RNBO_JuceAudioProcessorUtils.cpp
//
//  Created by Rob Sussman on 8/14/15.
//
//

#include "RNBO_JuceAudioProcessorUtils.h"

namespace RNBO {

namespace JuceAudioProcessorUtils {


void fillCurrentEventsFromMidiBuffer(EventList<EventVariant>& currentEvents,
									 const TimeConverter& timeConverter,
									 juce::MidiBuffer& midiMessages)
{
	if (!midiMessages.isEmpty()) {

		for (auto meta: midiMessages) {
			// take the MIDI events coming in via Juce and stuff them inside the list of _currentEvents (the list is sorted by time)
			MillisecondTime time = timeConverter.convertSampleOffsetToMilliseconds(meta.samplePosition);
			currentEvents.addEvent(MidiEvent(time, 0, meta.data, (Index)meta.numBytes));
		}

		// The JUCE AudioProcessor model is that any events left in the midiBuffer
		// are treated as the output of the plugin so we need to clear or else everything
		// gets sent on.
		midiMessages.clear();
	}
}

void fillAudioInputBuffers(const std::vector<SampleValue*>& audioInputs,
						   size_t sampleFrames,
						   juce::AudioSampleBuffer& buffer)
{
	int channelIndex = 0;
	for (SampleValue *pWritePtr : audioInputs)
	{
		if (channelIndex < buffer.getNumChannels())
		{
			const float *pReadPtr = buffer.getReadPointer(channelIndex);
			size_t framesLeft = sampleFrames;
			while (framesLeft--)
			{
				*pWritePtr++ = *pReadPtr++;
			}
		}
		else
		{
			memset(pWritePtr, 0, sampleFrames * sizeof(SampleValue));
		}
		channelIndex++;
	}
}

void fillAudioOutputBuffers(const std::vector<SampleValue*>& audioOutputs,
							size_t sampleFrames,
							juce::AudioSampleBuffer& buffer)
{
	int channelCount = buffer.getNumChannels();
	for (int channelIndex = 0; channelIndex < channelCount; channelIndex++)
	{
		float *pWritePtr = buffer.getWritePointer(channelIndex);
		if (channelIndex < (int)audioOutputs.size())
		{
			const SampleValue* pReadPtr = audioOutputs[(size_t)channelIndex];
			size_t framesLeft = sampleFrames;
			while (framesLeft--)
			{
				*pWritePtr++ = (float) *pReadPtr++;
			}
		}
		else
		{
			buffer.clear(channelIndex, 0, buffer.getNumSamples());
		}
	}
}

void sendOutgoingMidiEvents(EventList<MidiEvent>& midiEvents,
							MillisecondTime endTime,
							TimeConverter timeConverter,
							juce::MidiBuffer& midiMessages)
{
	if (!midiEvents.empty()) {
		const auto it = midiEvents.getFirstEventAfterTime(endTime);
		// now we have the range of events that are valid for
		std::for_each(midiEvents.begin(),
					  it,
					  [&timeConverter, &midiMessages](const MidiEvent& ev) {
						  int sampleNumber = static_cast<int>(timeConverter.convertMillisecondsToSampleOffset(ev.getTime()));
						  auto midiMessage = juce::MidiMessage(ev.getData(), (int)ev.getLength());
						  midiMessages.addEvent(midiMessage, sampleNumber);
					  });
		midiEvents.erase(midiEvents.begin(), it);
	}
}


} // namespace JuceAudioProcessorUtils

} // namespace RNBO
