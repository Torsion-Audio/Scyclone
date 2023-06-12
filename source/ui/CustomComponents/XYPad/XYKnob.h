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
    juce::Label* getLabel() {   return &nameLabel;   };

	Arrow* getOwnedArrow();
	ArrowButtons* getOwnedArrowButtons();

    void mouseDown (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseEnter(const juce::MouseEvent & event) override;

private:
	int index;
    bool isMouseDown = false;

	Arrow* ownedArrow;
	ArrowButtons* ownedArrowButton1;
	ArrowButtons* ownedArrowButton2;
	ArrowButtons* ownedArrowButton3;

	juce::Label nameLabel;
};

