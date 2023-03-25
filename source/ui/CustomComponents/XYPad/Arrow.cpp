#include "Arrow.h"

Arrow::Arrow(const juce::AudioProcessorValueTreeState& , int)
{
	arrowUpLeft = juce::Drawable::createFromImageData(BinaryData::arrowUpLeft_svg, BinaryData::arrowUpLeft_svgSize);
	arrowUpRight = juce::Drawable::createFromImageData(BinaryData::arrowUpRight_svg, BinaryData::arrowUpRight_svgSize);
	arrowDownLeft = juce::Drawable::createFromImageData(BinaryData::arrowDownLeft_svg, BinaryData::arrowDownLeft_svgSize);
	arrowDownRight = juce::Drawable::createFromImageData(BinaryData::arrowDownRight_svg, BinaryData::arrowDownRight_svgSize);

}

Arrow::~Arrow()
= default;

void Arrow::paint(juce::Graphics& g)
{
	// paint the arrow depending on its orientation which depends on its position
	switch (orientation)
	{
		case orientations::upLeft:
			arrowUpLeft->draw(g, 0.6f);
			break;
		case orientations::upRight:
			arrowUpRight->draw(g, 0.6f);
			break;
		case orientations::downLeft:
			arrowDownLeft->draw(g, 0.6f);
			break;
		case orientations::downRight:
			arrowDownRight->draw(g, 0.6f);
			break;
		default:
			break;
	}
}

void Arrow::resized()
{
}

void Arrow::setOrientation(int newOrientation)
{
	/* use the enum orientations*/
	orientation = newOrientation;
}

int Arrow::getOrientation() const
{
	return orientation;
}
