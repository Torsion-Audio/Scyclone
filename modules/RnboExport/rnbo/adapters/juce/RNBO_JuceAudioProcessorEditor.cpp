/*
  ==============================================================================

    RNBO_JuceAudioProcessorEditor.cpp
    Created: 21 Sep 2015 11:50:17am
    Author:  stb

  ==============================================================================
*/

#include "RNBO_JuceAudioProcessorEditor.h"

//TODO get rid of this
using namespace juce;

namespace RNBO {


class ProcessorParameterPropertyComp   : public PropertyComponent,
private AudioProcessorListener,
private Timer
{
	using String = juce::String;
public:
	ProcessorParameterPropertyComp (const String& name, AudioProcessor& p, int paramIndex)
	: PropertyComponent (name),
	owner (p),
	index (paramIndex),
	paramHasChanged (false),
	slider (p, paramIndex)
	{
		startTimer (100);
		addAndMakeVisible (slider);
		owner.addListener (this);
	}

	~ProcessorParameterPropertyComp() override
	{
		owner.removeListener (this);
	}

	void refresh() override
	{
		paramHasChanged = false;

		if (slider.getThumbBeingDragged() < 0)
			slider.setValue (owner.getParameters()[index]->getValue(), dontSendNotification);

		slider.updateText();
	}

	void audioProcessorChanged (AudioProcessor*, const ChangeDetails&) override { }

	void audioProcessorParameterChanged (AudioProcessor*, int parameterIndex, float) override
	{
		if (parameterIndex == index)
			paramHasChanged = true;
	}

	void notifyParamChange()
	{
		paramHasChanged = true;
	}

	void timerCallback() override
	{
		if (paramHasChanged)
		{
			refresh();
			startTimerHz (50);
		}
		else
		{
			startTimer (jmin (1000 / 4, getTimerInterval() + 10));
		}
	}

private:
	//==============================================================================
	class ParamSlider  : public Slider
	{
		using String = juce::String;
	public:
		ParamSlider (AudioProcessor& p, int paramIndex)  : owner (p), index (paramIndex)
		{
			const int steps = owner.getParameters()[index]->getNumSteps();

			if (steps > 1 && steps < 0x7fffffff)
				setRange (0.0, 1.0, 1.0 / (steps - 1.0));
			else
				setRange (0.0, 1.0);

			setSliderStyle (Slider::LinearBar);
			setTextBoxIsEditable (false);
			setScrollWheelEnabled (true);
		}

		void valueChanged() override
		{
			const float newVal = (float) getValue();
			const auto param = owner.getParameters()[index];

			if (param->getValue() != newVal)
			{
				param->beginChangeGesture();
				param->setValueNotifyingHost(newVal);
				param->endChangeGesture();
				updateText();
			}
		}

		String getTextFromValue (double /*value*/) override
		{
			return owner.getParameters()[index]->getCurrentValueAsText() + " " + owner.getParameters()[index]->getLabel().trimEnd();
		}

	private:
		//==============================================================================
		AudioProcessor& owner;
		const int index;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamSlider)
	};

	AudioProcessor& owner;
	const int index;
	bool volatile paramHasChanged;
	ParamSlider slider;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorParameterPropertyComp)
};

class PresetPropertyComp : public PropertyComponent, public ComboBox::Listener
{
    using String = juce::String;

public:
    PresetPropertyComp (AudioProcessor& p)
    : PropertyComponent("Preset"),
    owner (p),
    _comboBox("Presets")
    {
        _comboBox.clear();
        for (int i = 0; i < owner.getNumPrograms(); i++) {
            String name = owner.getProgramName(i);
            if (name.isEmpty()) continue;
            _comboBox.addItem(name, i + 1);
        }
        _comboBox.addListener(this);
        addAndMakeVisible (_comboBox);
    }

    ~PresetPropertyComp() override {
        _comboBox.removeListener(this);
    }

    void refresh() override {}

    void audioProcessorChanged (AudioProcessor*) {}

    void comboBoxChanged (ComboBox *comboBoxThatHasChanged) override {
        int newProgramIndex = comboBoxThatHasChanged->getSelectedId() - 1;
		owner.setCurrentProgram(newProgramIndex);
    }

private:
    AudioProcessor& owner;
    ComboBox _comboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetPropertyComp)
};

class DataRefPropertyComp : public PropertyComponent, Button::Listener
{
    using String = juce::String;

public:
    DataRefPropertyComp (const String& name, JuceAudioProcessor& p, RNBOAudioProcessorEditor * editor)
    : PropertyComponent(name), owner (p), _editor(editor), _label(), _button("load")
    {
			_button.addListener(this);
			_label.setText (p.loadedDataRefFile(name), juce::dontSendNotification);
			addAndMakeVisible (_label);
			addAndMakeVisible (_button);
    }

    ~DataRefPropertyComp() override { }

    void refresh() override {}

		void resized() override
		{
			auto bounds = getLocalBounds();
			auto width = bounds.getWidth();

			auto height = bounds.getHeight();
			juce::FlexBox fb;

			fb.flexDirection = juce::FlexBox::Direction::row;
			fb.flexWrap = juce::FlexBox::Wrap::noWrap;

			fb.items.add (juce::FlexItem (_label).withHeight(height).withFlex(2.0));
			fb.items.add (juce::FlexItem (_button).withHeight(height).withFlex(0.5));
			//layout includes Panel label, so we just use the right 1/2
			fb.performLayout(bounds.removeFromRight(width / 2).toFloat());
		}

    void audioProcessorChanged (AudioProcessor*) {}
		virtual void buttonClicked (juce::Button *) override {
			if (_editor) {
				_editor->chooseFileForDataRef(getName());
			}
		}

		void updateFileName(juce::String v) {
			_label.setText(v, juce::dontSendNotification);
		}

private:
    JuceAudioProcessor& owner;
		RNBOAudioProcessorEditor * _editor;
		Label _label;
		TextButton _button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataRefPropertyComp)
};

//==============================================================================
RNBOAudioProcessorEditor::RNBOAudioProcessorEditor(JuceAudioProcessor* const p, CoreObject& rnboObject)
: AudioProcessorEditor (p)
, _owner(p)
, _rnboObject(rnboObject)
, _parameterInterface(_rnboObject.createParameterInterface(ParameterEventInterface::SingleProducer, this))
{
	jassert (p != nullptr);
	setOpaque (true);
	p->addDataRefListener(this);

	addAndMakeVisible (_panel);

	const int numParams = p->getParameters().size();
	int totalHeight = 0;

	for (int i = 0; i < numParams; ++i)
	{
		juce::String name (p->getParameters()[i]->getName(256));
		if (name.trim().isEmpty())
			name = "Unnamed";

		ProcessorParameterPropertyComp* const pc = new ProcessorParameterPropertyComp (name, *p, i);
		_params.add (pc);
		totalHeight += pc->getPreferredHeight();
	}
	_panel.addProperties (_params);

	//datarefs
	for (auto index = 0; index < _rnboObject.getNumExternalDataRefs(); index++) {
		auto id = _rnboObject.getExternalDataId(index);
		juce::String name(id);
		auto d = new DataRefPropertyComp(name, *p, this);
		totalHeight += d->getPreferredHeight();
		_dataRefPropertyMap.insert({name, d});
		_panel.addProperties(d);
	}

	//add presets if we have them
	{
		bool hasPresets = false;
		for (int i = 0; i < p->getNumPrograms(); i++) {
			juce::String name = p->getProgramName(i);
			if (!name.isEmpty()) {
				hasPresets = true;
				break;
			}
		}
		if (hasPresets) {
			PresetPropertyComp* const presetChooserComponent = new PresetPropertyComp (*p);
			totalHeight += presetChooserComponent->getPreferredHeight();
			_panel.addProperties (presetChooserComponent);
		}
	}

	setSize (400, jlimit (25, 400, totalHeight));
}

RNBOAudioProcessorEditor::~RNBOAudioProcessorEditor()
{
}

void RNBOAudioProcessorEditor::paint (Graphics& g)
{
	g.fillAll (Colours::white);
}

void RNBOAudioProcessorEditor::resized()
{
	_panel.setBounds (getLocalBounds());
}

void RNBOAudioProcessorEditor::handleAsyncUpdate()
{
	drainEvents();
}

void RNBOAudioProcessorEditor::eventsAvailable()
{
	this->triggerAsyncUpdate();
}

void RNBOAudioProcessorEditor::updateAllParams()
{
	for (PropertyComponent* p : _params) {
		ProcessorParameterPropertyComp* pc = dynamic_cast<ProcessorParameterPropertyComp*>(p);
		pc->notifyParamChange();
	}
}

void RNBOAudioProcessorEditor::handleParameterEvent(const ParameterEvent&) {
	updateAllParams();
}

void RNBOAudioProcessorEditor::handlePresetEvent(const PresetEvent& event) {
	if (event.getType() == RNBO::PresetEvent::Touched) {
		updateAllParams();
	}
}

void RNBOAudioProcessorEditor::chooseFileForDataRef(const juce::String refName) {
	_dataRefChooser = std::make_unique<juce::FileChooser> ("Select an audio file to load..");

	auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

	_dataRefChooser->launchAsync (folderChooserFlags, [this, refName] (const FileChooser& chooser)
	{
		auto file = chooser.getResult();
		if (file.exists()) {
			_owner->loadDataRef(refName, file);
		}
	});
}

void RNBOAudioProcessorEditor::handleMessage(const Message& message) {
	if (auto* m = dynamic_cast<const DataRefUpdatedMessage*>(&message)) {
		auto it = _dataRefPropertyMap.find(m->refName());
		if (it != _dataRefPropertyMap.end()) {
			it->second->updateFileName(m->fileName());
		}
	}
}

} // namespace RNBO
