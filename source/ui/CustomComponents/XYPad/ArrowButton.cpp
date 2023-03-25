#include "ArrowButton.h"

ArrowButtons::ArrowButtons(int type, juce::RangedAudioParameter& param):
                                                                        buttonType(type),
                                                                        juce::DrawableButton("Test", juce::DrawableButton::ButtonStyle::ImageStretched),
                                                                        buttonAttachment(param, *this, nullptr)

{
	setToggleable(true);
	setClickingTogglesState(true);


	knobButton1_normal = juce::Drawable::createFromImageData(BinaryData::knobButton1normal_svg, BinaryData::knobButton1normal_svgSize);
	knobButton1_over = juce::Drawable::createFromImageData(BinaryData::knobButton1over_svg, BinaryData::knobButton1over_svgSize);
	knobButton1_down = juce::Drawable::createFromImageData(BinaryData::knobButton1down_svg, BinaryData::knobButton1down_svgSize);

	knobButton2_normal = juce::Drawable::createFromImageData(BinaryData::knobButton2normal_svg, BinaryData::knobButton2normal_svgSize);
	knobButton2_over = juce::Drawable::createFromImageData(BinaryData::knobButton2over_svg, BinaryData::knobButton2over_svgSize);
	knobButton2_down = juce::Drawable::createFromImageData(BinaryData::knobButton2down_svg, BinaryData::knobButton2down_svgSize);

	knobButton3_normal = juce::Drawable::createFromImageData(BinaryData::knobButton3normal_svg, BinaryData::knobButton3normal_svgSize);
	knobButton3_over = juce::Drawable::createFromImageData(BinaryData::knobButton3over_svg, BinaryData::knobButton3over_svgSize);
	knobButton3_down = juce::Drawable::createFromImageData(BinaryData::knobButton3down_svg, BinaryData::knobButton3down_svgSize);

    this->setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentWhite);

	switch (buttonType)
	{
	case 1:
		setImages(knobButton1_normal.get(),
			knobButton1_over.get(),
			knobButton1_down.get(),
			nullptr,
			knobButton1_down.get());
		break;
	case 2:
		setImages(knobButton2_normal.get(),
			knobButton2_over.get(),
			knobButton2_down.get(),
			nullptr,
			knobButton2_down.get());
		break;
	case 3:
		setImages(knobButton3_normal.get(),
			knobButton3_over.get(),
			knobButton3_down.get(),
			nullptr,
			knobButton3_down.get());
		break;
	default:
		break;
	}
    //buttonAttachment.sendInitialUpdate();
}
ArrowButtons::~ArrowButtons()
= default;

int ArrowButtons::getType() const
{
	return buttonType;
}
