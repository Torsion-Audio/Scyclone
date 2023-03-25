#include "XYKnob.h"

XYKnob::XYKnob(const juce::AudioProcessorValueTreeState& , int idx, Arrow& a, ArrowButtons& b1, ArrowButtons& b2, ArrowButtons& b3) : index(idx)
{
    addMouseListener(this, true);
    juce::ignoreUnused(index);

	ownedArrow = &a;
	ownedArrowButton1 = &b1;
	ownedArrowButton2 = &b2;
	ownedArrowButton3 = &b3;

	nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::KNOB_LABEL).withAlpha(0.75f));
	nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(CustomFontLookAndFeel::getCustomFontBold().withHeight(16));

	if (idx == 1)
	{
		//nameLabel.setText("Voice", juce::dontSendNotification);
		nameLabel.setComponentID("nameLabel1");
	}
	else if (idx == 2)
	{
		//nameLabel.setText("Acid", juce::dontSendNotification);
		nameLabel.setComponentID("nameLabel2");
	}
    addAndMakeVisible(nameLabel);
}

XYKnob::~XYKnob()
= default;

void XYKnob::paint(juce::Graphics& g)
{
	g.setColour(juce::Colour::fromString(ColorPallete::KNOB));
	g.fillEllipse(getLocalBounds().toFloat());
    if (isMouseDown)
    {
        g.setColour(juce::Colours::aquamarine);
        g.drawEllipse(1.f, 1.f, (float) getWidth() - 2.f, (float) getHeight() - 2.f, 2.f);
    }

    scycloneLogo->drawWithin(g, getLocalBounds().toFloat(), juce::RectanglePlacement::centred, 0);
}
void XYKnob::resized()
{
	auto labelWidth = nameLabel.getWidth();
	auto labelHeight = nameLabel.getHeight();
    juce::ignoreUnused(labelWidth, labelHeight);

	nameLabel.setBounds(getLocalBounds());
}

void XYKnob::setName(juce::String& name)
{
	nameLabel.setText(name, juce::dontSendNotification);
	repaint();
}
juce::String XYKnob::getLabelID()
{
	return nameLabel.getComponentID();
}
Arrow* XYKnob::getOwnedArrow()
{
	return ownedArrow;
}
ArrowButtons* XYKnob::getOwnedArrowButtons()
{
	static ArrowButtons* buttons[3];
	buttons[0] = ownedArrowButton1;
	buttons[1] = ownedArrowButton2;
	buttons[2] = ownedArrowButton3;
	return *buttons;
}

void XYKnob::mouseDown(const juce::MouseEvent &)
{
    isMouseDown = true;
    repaint();
    DBG("knob clicked");
}

void XYKnob::mouseUp(const juce::MouseEvent &)
{
    isMouseDown = false;
    repaint();
}
/*
void XYKnob::connectXtoParameter(juce::RangedAudioParameter& p)
{
	xAttachment = std::make_unique<juce::ParameterAttachment>(p, [this](float newValue)
		{

		});
	xAttachment->sendInitialUpdate();
}

void XYKnob::connectYtoParameter(juce::RangedAudioParameter& p)
{
	yAttachment = std::make_unique<juce::ParameterAttachment>(p, [this](float newValue)
		{

		});
	yAttachment->sendInitialUpdate();
}
*/