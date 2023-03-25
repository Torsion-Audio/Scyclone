#pragma once
#include "JuceHeader.h"
#include "../../../utils/colors.h"
#include "XYKnob.h"
#include "Arrow.h"
#include "ArrowButton.h"
#include "../../../PluginParameters.h"
#include "../../../PluginProcessor.h"

class XYPad : public juce::Component, public juce::Timer, private juce::AudioProcessorValueTreeState::Listener
{
private:

public:
    explicit XYPad(juce::AudioProcessorValueTreeState& parameters, AudioPluginAudioProcessor& p);
	~XYPad() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

    std::function<void(float, float, int)>onButtonMove;
    std::function<void(float)>onModelMixChange;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent&) override;


    void connectParameters(juce::RangedAudioParameter& x1,
						   juce::RangedAudioParameter& y1, 
						   juce::RangedAudioParameter& x2, 
						   juce::RangedAudioParameter& y2);



	float pixelToNormWidth(int pixelValue);
	float pixelToNormHeight(int pixelValue);
	int normToPixelWidth(float normValue);
	int normToPixelHeight(float normValue);

    void timerCallback() override;

    bool isButton1OnHover(const juce::MouseEvent &e);
    bool isButton2OnHover(const juce::MouseEvent &e);
    void setKnobButtonDiameter(int newDiameter, int buttonNumber);
    void moveButton(XYKnob* knob, int knobNumber);
    int calcCurrentButtonDiameters(int buttonNumber);
    void moveArrow(Arrow* arrow, float xNorm, float yNorm, int xPixel, int yPixel, int knobButtonDiameter);
    void moveArrowButton(ArrowButtons* arrowButton, Arrow* arrow) const;
    bool shouldKnobBeMuted(int knobNumber);
    void updateKnobName(int modelID, juce::String modelName) {
        if (modelID == 1) {
            processor.network1Name = modelName;
            knob1.setName(modelName);
        }
        else if (modelID == 2) {
            processor.network2Name = modelName;
            knob2.setName(modelName);
        }
    }

    const int knobButtonNeutralDiameter = 108;

private:

	juce::AudioProcessorValueTreeState& parameters;
    AudioPluginAudioProcessor& processor;
	std::unique_ptr<juce::Drawable> octagon;
		
	XYKnob knob1;
	Arrow arrow1;
	ArrowButtons network1SelectButton;
	ArrowButtons network1GrainButton;
	ArrowButtons network1OnOffButton;

	XYKnob knob2;
	Arrow arrow2;
	ArrowButtons network2SelectButton;
	ArrowButtons network2GrainButton;
	ArrowButtons network2OnOffButton;

    int arrowWidth = 165;
	int arrowHeight = 55;
	int arrowButtonDiameter = 18;

	std::unique_ptr<juce::ParameterAttachment> knob1XAttachment;
	std::unique_ptr<juce::ParameterAttachment> knob1YAttachment;
	std::unique_ptr<juce::ParameterAttachment> knob2XAttachment;
	std::unique_ptr<juce::ParameterAttachment> knob2YAttachment;

    const juce::String network1Name;
    const juce::String network2Name;

    /*
	std::unique_ptr<juce::ParameterAttachment> knob1Button1Attachment;
	std::unique_ptr<juce::ParameterAttachment> knob1Button2Attachment;
	std::unique_ptr<juce::ParameterAttachment> knob1Button3Attachment;
	std::unique_ptr<juce::ParameterAttachment> knob2Button1Attachment;
	std::unique_ptr<juce::ParameterAttachment> knob2Button2Attachment;
	std::unique_ptr<juce::ParameterAttachment> knob2Button3Attachment;
    */
	enum KnobOnDrag { noKnobOnDrag, knob1OnDrag, knob2OnDrag};
    int knobOnDrag{};
    enum KnobOnHover { noKnobOnHover, knob1OnHover, knob2OnHover};
    int knobOnHover = KnobOnHover::noKnobOnHover;

	enum Quadrant {upLeft, upRight, downLeft, downRight};


    void setArrowAndButtonsVisible(bool newState, int buttonNumber);
    int timerCounter = 0;

    std::unique_ptr<juce::ComponentAnimator> componentAnimator;
};

