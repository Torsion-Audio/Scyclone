/*
 ==============================================================================

 RNBO_JuceAudioProcessorEditor.h
 Created: 21 Sep 2015 11:50:17am
 Author:  stb

 ==============================================================================
 */

#ifndef RNBO_JUCEAUDIOPROCESSOREDITOR_H_INCLUDED
#define RNBO_JUCEAUDIOPROCESSOREDITOR_H_INCLUDED

#include "RNBO.h"
#include "RNBO_JuceAudioProcessor.h"
#include <unordered_map>

namespace RNBO {
	//forward decl
	class DataRefPropertyComp;

	//==============================================================================
	/**

		this is mostly a copy of a Juce GenericAudioProcessorEditor, with the important
		difference that it can be refreshed via a ParameterInterfacHandle

	 */
	class RNBOAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::AsyncUpdater, public RNBO::EventHandler, public juce::MessageListener
	{
	public:
		//==============================================================================
		RNBOAudioProcessorEditor (JuceAudioProcessor* owner, CoreObject& rnboObject);
		~RNBOAudioProcessorEditor() override;

		//==============================================================================
		void paint (juce::Graphics&) override;
		void resized() override;

		void handleAsyncUpdate() override;

		void eventsAvailable() override;
		void handleParameterEvent(const RNBO::ParameterEvent& event) override;
		void handlePresetEvent(const RNBO::PresetEvent& event) override;

		void chooseFileForDataRef(const juce::String refName);
		JuceAudioProcessor * owner() const { return _owner; }

		void handleMessage(const juce::Message& message) override;

	private:

		void updateAllParams();

		//==============================================================================
		JuceAudioProcessor* _owner;
		juce::PropertyPanel							_panel;
		CoreObject&								_rnboObject;
		ParameterEventInterfaceUniquePtr		_parameterInterface;

		juce::Array <juce::PropertyComponent*>	_params;
		std::unique_ptr<juce::FileChooser> _dataRefChooser;
		std::unordered_map<juce::String, DataRefPropertyComp*> _dataRefPropertyMap;
	};

} // namespace RNBO

#endif  // RNBO_JUCEAUDIOPROCESSOREDITOR_H_INCLUDED
