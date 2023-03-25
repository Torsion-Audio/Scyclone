#pragma once
#include "JuceHeader.h"
#include "../../../utils/colors.h"

class   ArrowButtons : public juce::DrawableButton
{
public:
	ArrowButtons(int type, juce::RangedAudioParameter& param);
	~ArrowButtons() override;

	int getType() const;

private:
	int buttonType; // which of the three button types (1-3)

    juce::ButtonParameterAttachment buttonAttachment;

	std::unique_ptr<juce::Drawable> knobButton1_normal;
	std::unique_ptr<juce::Drawable> knobButton1_over;
	std::unique_ptr<juce::Drawable> knobButton1_down;

	std::unique_ptr<juce::Drawable> knobButton2_normal;
	std::unique_ptr<juce::Drawable> knobButton2_over;
	std::unique_ptr<juce::Drawable> knobButton2_down;

	std::unique_ptr<juce::Drawable> knobButton3_normal;
	std::unique_ptr<juce::Drawable> knobButton3_over;
	std::unique_ptr<juce::Drawable> knobButton3_down;
};

