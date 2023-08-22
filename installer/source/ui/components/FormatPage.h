//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_FORMATPAGE_H
#define GUI_APP_EXAMPLE_FORMATPAGE_H

#include <JuceHeader.h>
#include "template/TemplateInterfacePage.h"
#include "../../utils/FormatOptions.h"

class FormatPage : public TemplateInterfacePage {
public:
    FormatPage();
    std::vector<FormatOptions::Options> getSelectedOptions();

private:
    void formatSelectionChanged();
    void resized() override;

private:
    juce::Label headerLabel {"headerLabel", "Options"};

    juce::ToggleButton vst3Format {"VST3 Audio Plugin"};
    juce::ToggleButton standaloneFormat {"Standalone Version"};
#if JUCE_WINDOWS
    std::array<juce::ToggleButton*, 2> formatOptions {&vst3Format, &standaloneFormat};
    std::vector<FormatOptions::Options> selectedOptions {FormatOptions::Options::VST3Plugin,
                                                         FormatOptions::Options::Standalone};
#elif JUCE_MAC
    juce::ToggleButton auFormat {"AU Audio Plugin"};
    std::array<juce::ToggleButton*, 3> formatOptions {&vst3Format, &standaloneFormat, &auFormat};
    std::vector<FormatOptions::Options> selectedOptions {FormatOptions::Options::VST3Plugin,
                                                         FormatOptions::Options::Standalone,
                                                         FormatOptions::Options::AUPlugin};
#endif
    CustomButton button {"Next"};
};



#endif //GUI_APP_EXAMPLE_FORMATPAGE_H
