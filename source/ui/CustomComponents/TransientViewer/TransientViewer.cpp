#include "TransientViewer.h"

TransientViewer::TransientViewer(AudioPluginAudioProcessor& p) : waveViewerNetwork1(p.getAudioVisualiser1()),
                                                                 waveViewerNetwork2(p.getAudioVisualiser2())
{
    addAndMakeVisible(waveViewerNetwork1);
    addAndMakeVisible(waveViewerNetwork2);
    addAndMakeVisible(transientGrid);
    auto bgcolor = juce::Colour::fromString(ColorPallete::BG);

    auto wavefromColour1 = juce::Colour{0xff8172A3};
    auto wavefromColour2 = juce::Colour{0xffBA3B4D};

    waveViewerNetwork1.setColours(bgcolor, wavefromColour1);
    waveViewerNetwork2.setColours(bgcolor, wavefromColour2);
}

TransientViewer::~TransientViewer()
= default;

void TransientViewer::paint(juce::Graphics& )
{
}

void TransientViewer::resized()
{
    auto area = getLocalBounds().removeFromTop(static_cast<int>((float) getHeight()*0.97f));

    area.removeFromRight(2);

    auto area1 = area.removeFromTop(area.getHeight()/2);
    auto area2 = area;

    waveViewerNetwork1.setBounds(area1.reduced(5));
    waveViewerNetwork2.setBounds(area2.reduced(5));
    transientGrid.setBounds(getLocalBounds());
}
