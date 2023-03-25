//
// Created by valentin.ackva on 31.01.2023.
//

#ifndef VAESYNTH_CUSTOMSLIDERCOMPONENT_H
#define VAESYNTH_CUSTOMSLIDERCOMPONENT_H

#include "JuceHeader.h"
#include "CustomComponents/CustomSlider.h"
#include "CustomComponents/CustomLabel.h"
#include "../../../LookAndFeel/SliderLookAndFeel.h"
#include "../../../LookAndFeel/CustomFontLookAndFeel.h"

class CustomSliderComponent : public juce::Component {
public:
    explicit CustomSliderComponent(juce::String sliderName = "Test", CustomSliderType type = normal);
    ~CustomSliderComponent() override;

    void addSliderAttachment(juce::AudioProcessorValueTreeState& state, const juce::String& parameterID);
    void setCustomColour(CustomSliderColourID colourId, juce::Colour colour);
    void setDoubleClickReturnValue(double valueToSetOnDoubleClick);

private:
    void sliderChanged();
    void openManualValueBox ();
    void manuelValueBoxChanged();
    void updateValueLabel();
    void resized() override;

private:
    SliderLookAndFeel customSliderLookAndFeel;
    CrossfadeSliderLookAndFeel customCrossfadeSliderLookAndFeel;

    CustomSliderType sliderType;

    juce::Label titelLabel;
    CustomLabel valueLabel;

    CustomSlider slider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    std::unique_ptr<juce::RangedAudioParameter*> parameter;

    juce::Colour textColour {0xff868686};
    juce::Colour valueColour {0xffE2D1C3};
};

#endif //VAESYNTH_CUSTOMSLIDERCOMPONENT_H
