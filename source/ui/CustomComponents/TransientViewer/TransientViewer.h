#include <JuceHeader.h>
#include "../../../utils/colors.h"
#include "../../../PluginProcessor.h"
#include "TransientGrid.h"

class TransientViewer : public juce::Component
{
public:
	explicit TransientViewer(AudioPluginAudioProcessor& p);
	~TransientViewer();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
    juce::AudioVisualiserComponent& waveViewerNetwork1;
    juce::AudioVisualiserComponent& waveViewerNetwork2;
    TransientGrid transientGrid;
};