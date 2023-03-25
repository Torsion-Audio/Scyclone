//
// Created by valentin.ackva on 24.02.2023.
//

#ifndef VAESYNTH_TEXTURECOMPONENT_H
#define VAESYNTH_TEXTURECOMPONENT_H

#include "JuceHeader.h"

class TextureComponent : public juce::Component {
public:
    TextureComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::unique_ptr<juce::Drawable> background = juce::Drawable::createFromImageData (BinaryData::Gradient_png, BinaryData::Gradient_pngSize);
};

class OpenGLTextureComponent : public juce::Component {
public:
    OpenGLTextureComponent();

    void paint(juce::Graphics& g) override;

private:
    std::unique_ptr<juce::Drawable> background = juce::Drawable::createFromImageData (BinaryData::Gradient_OpenGL_png, BinaryData::Gradient_OpenGL_pngSize);
};

#endif //VAESYNTH_TEXTURECOMPONENT_H
