//
// Created by valentin.ackva on 06.07.2023.
//

#include "CustomButton.h"

CustomButton::CustomButton(juce::String buttonText) : button("button", juce::DrawableButton::ButtonStyle::ImageFitted) {
    button.setImages(buttonOff.get(),
                     buttonOff.get(),
                     buttonOn.get(),
                     buttonOff.get(),
                     buttonOn.get(),
                     buttonOn.get(),
                     buttonOn.get(),
                     buttonOn.get());
    button.onClick = [this] { buttonClicked(); };
    addAndMakeVisible(button);

    buttonLabel.setText(buttonText, juce::dontSendNotification);
    buttonLabel.setJustificationType(juce::Justification::centred);
    buttonLabel.setEditable(false, false, false);
    buttonLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(buttonLabel);
}

void CustomButton::addListener(CustomButton::Listener *listener) {
    listeners.add(listener);
}

void CustomButton::removeListener(CustomButton::Listener *listener) {
    listeners.remove(listener);
}

void CustomButton::setButtonText(juce::String text) {
    buttonLabel.setText(text, juce::dontSendNotification);
}

juce::String CustomButton::getButtonText() {
    return buttonLabel.getText();
}

void CustomButton::buttonClicked() {
    listeners.call([](Listener& l) { l.buttonClicked(); });
}

void CustomButton::resized() {
    button.setBounds(getLocalBounds());
    buttonLabel.setBounds(getLocalBounds());
}
