#pragma once
#include "JuceHeader.h"
#include "../../../utils/colors.h"
#include "../../LookAndFeel/CustomFontLookAndFeel.h"
#include "Arrow.h"
#include "ArrowButton.h"


class XYKnob : public juce::Component
{
public:
	XYKnob(const juce::AudioProcessorValueTreeState& apvts, int idx, Arrow& a, ArrowButtons& b1, ArrowButtons& b2, ArrowButtons& b3);
	~XYKnob() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setName(juce::String& name);

	juce::String getLabelID();

	Arrow* getOwnedArrow();
	ArrowButtons* getOwnedArrowButtons();

    void mouseDown (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;


	//void connectXtoParameter(juce::RangedAudioParameter& p);
	//void connectYtoParameter(juce::RangedAudioParameter& p);

private:
	int index;
    bool isMouseDown = false;

	Arrow* ownedArrow;
	ArrowButtons* ownedArrowButton1;
	ArrowButtons* ownedArrowButton2;
	ArrowButtons* ownedArrowButton3;

    std::unique_ptr<juce::Drawable> scycloneLogo = juce::Drawable::createFromImageData(BinaryData::Logo_svg, BinaryData::Logo_svgSize);

    /*
	juce::Rectangle<float> knobSection;	
	juce::Rectangle<float> arrowSection;
	juce::Rectangle<float> knobButton1Section;
	juce::Rectangle<float> knobButton2Section;
	juce::Rectangle<float> knobButton3Section;
	*/

	juce::Label nameLabel;

	/*
	std::unique_ptr<juce::ParameterAttachment> xAttachment;
	std::unique_ptr<juce::ParameterAttachment> yAttachment;
	*/
};

