//
// Created by valentin.ackva on 16.03.2023.
//

#include "TextureComponent.h"

TextureComponent::TextureComponent() {
    this->setInterceptsMouseClicks(false, false);
}

void TextureComponent::paint(juce::Graphics &g) {
    background->paintEntireComponent(g, false);
}

void TextureComponent::resized() {
    background->setBounds(getLocalBounds());
}

OpenGLTextureComponent::OpenGLTextureComponent() {
    this->setInterceptsMouseClicks(false, false);
}

void OpenGLTextureComponent::paint(juce::Graphics &g) {
    background->setBounds(getLocalBounds());
    background->paintEntireComponent(g, false);
}
