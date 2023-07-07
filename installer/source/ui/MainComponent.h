#pragma once

#include "JuceHeader.h"


#include "components/customComponent/CustomLabel.h"
#include "components/WelcomePage.h"
#include "components/template/TemplateInterfacePage.h"
#include "components/FormatPage.h"
#include "components/PathPage.h"
#include "components/ProcessingPage.h"
#include "components/EndPage.h"

#include "../operations/OperationManager.h"
#include "lookAndFeel/InstallerLookAndFeel.h"

class MainComponent : public juce::Component, private TemplateInterfacePage::Listener, private juce::Timer
{
public:
    MainComponent(OperationManager& ref);
    ~MainComponent();

private:
    void paint (juce::Graphics&) override;
    void resized() override;

    void pageFinished(TemplateInterfacePage* page) override;
    void cancelProcess(TemplateInterfacePage* page) override;

    void startInstallProcess();
    void timerCallback() override;

private:
    OperationManager& operationManagerRef;

    WelcomePage welcomePage;
    FormatPage formatPage;
    PathPage pathPage;
    ProcessingPage processingPage;
    EndPage endPage;

    std::unique_ptr<juce::Drawable> background = juce::Drawable::createFromImageData (BinaryData::background_svg, BinaryData::background_svgSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
