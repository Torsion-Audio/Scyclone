#include "Main.h"

GuiAppApplication::GuiAppApplication() {}

const juce::String GuiAppApplication::getApplicationName() {
    return JUCE_APPLICATION_NAME_STRING;
}

const juce::String GuiAppApplication::getApplicationVersion() {
    return JUCE_APPLICATION_VERSION_STRING;
}

bool GuiAppApplication::moreThanOneInstanceAllowed() {
    return false;
}

void GuiAppApplication::initialise(const String &commandLine) {
    juce::ignoreUnused (commandLine);

    mainWindow.reset (new MainWindow (getApplicationName(), installManager));
    mainWindow->setUsingNativeTitleBar(true);
}

void GuiAppApplication::shutdown() {
    mainWindow = nullptr;
}

void GuiAppApplication::systemRequestedQuit() {
    quit();
}

void GuiAppApplication::anotherInstanceStarted(const String &commandLine) {
    juce::ignoreUnused (commandLine);
}
