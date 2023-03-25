#pragma once
#include "JuceHeader.h"
#include "../../../utils/colors.h"
#include "ArrowButton.h"

class Arrow : public juce::Component
{
public:
	Arrow(const juce::AudioProcessorValueTreeState& parameters, int idx);
	~Arrow();

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setOrientation(int newOrientation);
	int getOrientation() const;
	enum orientations { upLeft, upRight, downLeft, downRight };


private:
	std::unique_ptr<juce::Drawable> arrowUpLeft;
	std::unique_ptr<juce::Drawable> arrowUpRight;
	std::unique_ptr<juce::Drawable> arrowDownLeft;
	std::unique_ptr<juce::Drawable> arrowDownRight;

	int orientation = 1;
};


