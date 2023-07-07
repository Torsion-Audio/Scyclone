//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_CUSTOMBUTTON_H
#define GUI_APP_EXAMPLE_CUSTOMBUTTON_H

#include "JuceHeader.h"

class CustomButton : public juce::Component {
public:
    CustomButton(juce::String buttonText);

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void buttonClicked() = 0;
    };

    virtual void addListener(Listener* listener);
    virtual void removeListener(Listener* listener);

    void setButtonText(juce::String text);
    juce::String getButtonText();

private:
    void buttonClicked();
    void resized() override;

private:
    juce::DrawableButton button;
    juce::Label buttonLabel;

    juce::ListenerList<Listener> listeners;

    std::unique_ptr<juce::Drawable> buttonOn = juce::Drawable::createFromImageData (BinaryData::button_on_svg,
                                                                                    BinaryData::button_on_svgSize);
    std::unique_ptr<juce::Drawable> buttonOff = juce::Drawable::createFromImageData (BinaryData::button_off_svg,
                                                                                     BinaryData::button_off_svgSize);
};


#endif //GUI_APP_EXAMPLE_CUSTOMBUTTON_H
