//
// Created by valentin.ackva on 12.02.2023.
//

#include "CustomLabel.h"

juce::TextEditor* CustomLabel::createEditorComponent() {
    std::unique_ptr<juce::TextEditor> ed;
    ed = std::make_unique<juce::TextEditor>(getName());
    //auto* ed = new juce::TextEditor (getName()); // exchanged with unique_ptr
    ed->applyFontToAllText (CustomFontLookAndFeel::getCustomFont());
    copyAllExplicitColoursTo (*ed);

    ed->setColour(juce::TextEditor::ColourIds::highlightColourId, juce::Colours::transparentWhite);
    ed->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentWhite);
    ed->setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    ed->setColour(juce::TextEditor::textColourId, juce::Colours::whitesmoke);
    ed->setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::whitesmoke);
    ed->setJustification(juce::Justification::centred);
    return ed.get();
}