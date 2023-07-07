#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(OperationManager& ref) : operationManagerRef(ref){
        setSize (500, 650);

        welcomePage.addListener(this);
        addAndMakeVisible(welcomePage);

        formatPage.addListener(this);
        addAndMakeVisible(formatPage);
        formatPage.setVisible(false);

        pathPage.addListener(this);
        addAndMakeVisible(pathPage);
        pathPage.setVisible(false);

        processingPage.addListener(this);
        addAndMakeVisible(processingPage);
        processingPage.setVisible(false);

        endPage.addListener(this);
        addAndMakeVisible(endPage);
        endPage.setVisible(false);

}

MainComponent::~MainComponent() {

}

//==============================================================================
void MainComponent::paint (juce::Graphics& g) {
    background->setBounds(getLocalBounds());
    background->paintEntireComponent(g, false);
}

void MainComponent::resized() {
    juce::Rectangle<int> pageBounds (0, 250, 500, 350);

    welcomePage.setBounds(pageBounds);
    formatPage.setBounds(pageBounds);
    pathPage.setBounds(pageBounds);
    processingPage.setBounds(pageBounds);
    endPage.setBounds(pageBounds);
}

void MainComponent::pageFinished(TemplateInterfacePage* page) {
    if (page == &welcomePage) {
        welcomePage.setVisible(false);
        formatPage.setVisible(true);
    } else if (page == &formatPage) {
        formatPage.setVisible(false);
        auto selected = formatPage.getSelectedOptions();
        pathPage.setFormatOptions(selected);
        pathPage.setVisible(true);
    } else if (page == &pathPage) {
        pathPage.setVisible(false);
        startInstallProcess();
    }
}

void MainComponent::startInstallProcess() {
    processingPage.setVisible(true);
    auto results = pathPage.getPathResults();
    operationManagerRef.configure(results);
    operationManagerRef.start();

    startTimerHz(30);
}

void MainComponent::cancelProcess(TemplateInterfacePage* page) {
    juce::ignoreUnused(page);

    formatPage.setVisible(false);
    pathPage.setVisible(false);
    processingPage.setVisible(false);
    endPage.setVisible(false);

    welcomePage.setVisible(true);

    operationManagerRef.stop();
}

void MainComponent::timerCallback() {
    auto operationState = operationManagerRef.getState();
    if (operationState == InstallationState::GitHubAccessed) {
        processingPage.setDownloadState(true);
        processingPage.setDownloadProgress(operationManagerRef.getDownloadProgress());
    } else if (operationState == InstallationState::InstallationFinished) {
        stopTimer();
        processingPage.setVisible(false);
        endPage.setVisible(true);
    } else {
        processingPage.setDownloadState(false);
    }
}