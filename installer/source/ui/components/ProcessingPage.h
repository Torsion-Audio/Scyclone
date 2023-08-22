//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_PROCESSINGPAGE_H
#define GUI_APP_EXAMPLE_PROCESSINGPAGE_H

#include <JuceHeader.h>
#include "template/TemplateInterfacePage.h"

class LoadingAnimation : public juce::AnimatedAppComponent {
public:
    LoadingAnimation()
    {
        setFramesPerSecond (60);
    }

    void setState(bool download) {
        downloading = download;
    }

    void setProgress(int progress) {
        downloadProgress = progress;
    }

private:
    void update() override
    {

    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour{0xff131214});

        g.setColour (juce::Colours::white);

        auto numberOfDots = 15;

        juce::Path spinePath;

        for (auto i = 0; i < numberOfDots; ++i)
        {
            int radius = getWidth() / 2 - 10;

            juce::Point<float> p ((float) getWidth()  / 2.0f + 1.0f * (float) radius * - std::sin ((float) getFrameCounter() * 0.04f + (float) i * 0.12f),
                                  (float) getHeight() / 2.0f + 1.0f * (float) radius * std::cos ((float) getFrameCounter() * 0.04f + (float) i * 0.12f));

            if (i == 0)
                spinePath.startNewSubPath (p);
            else
                spinePath.lineTo (p);
        }

        g.strokePath (spinePath, juce::PathStrokeType (5.0f));
        g.setColour (juce::Colours::whitesmoke);

        g.drawEllipse(getLocalBounds().reduced(10).toFloat(), 2.f);

        if (downloading) {
            g.drawText(juce::String{"downloading..."}, getLocalBounds(), juce::Justification::centred);

            juce::Rectangle<int> progressRect = getLocalBounds();
            progressRect.removeFromTop(70);
            g.drawText(juce::String{downloadProgress} + juce::String{" %"}, progressRect, juce::Justification::centred);
        } else {
            g.drawText(juce::String{"installing..."}, getLocalBounds(), juce::Justification::centred);
        }
    }
    void resized() override
    {
        setSize(getWidth(), getHeight());
    }

private:
    bool downloading = true;
    int downloadProgress = 0;
};

class ProcessingPage : public TemplateInterfacePage {
public:
    ProcessingPage()
    {
        addAndMakeVisible(loadingAnimation);
        initComponent(&button);
    }

    void setDownloadState(bool download) {
        loadingAnimation.setState(download);
    }

    void setDownloadProgress(int progress) {
        loadingAnimation.setProgress(progress);
    }

private:
    void buttonClicked() override {
        if (button.getButtonText() == "Cancel") {
            notifyCancel(this);
        } else {
            notifyFinished(this);
        }
    }

    void resized() override
    {
        int size = 160;
        loadingAnimation.setBounds((500-size) / 2, size / 4, size, size);
        button.setBounds(0, 250, 500, 50);
    }

private:
    LoadingAnimation loadingAnimation;
    CustomButton button {"Cancel"};


};


#endif //GUI_APP_EXAMPLE_PROCESSINGPAGE_H
