//
// Created by valentin.ackva on 07.07.2023.
//

#include "CustomLabel.h"

CustomLabel::CustomLabel(const String &componentText, const String &labelText) : juce::Label(componentText, labelText){

}

TextEditor *CustomLabel::createEditorComponent() {
    auto* ed = new TextEditor (getName());
    ed->applyFontToAllText (getLookAndFeel().getLabelFont (*this));
    copyAllExplicitColoursTo (*ed);
    ed->setJustification(juce::Justification::centredLeft);
    ed->setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    ed->setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, juce::Colours::transparentBlack);
    ed->setColour(juce::TextEditor::ColourIds::shadowColourId, juce::Colours::transparentBlack);
    ed->setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colour{0xff131214});
    ed->setColour(juce::TextEditor::ColourIds::textColourId , juce::Colour{0xff565656});
    ed->setColour(juce::TextEditor::ColourIds::highlightedTextColourId , juce::Colour{0xff565656});
    ed->setColour(juce::TextEditor::ColourIds::highlightColourId , juce::Colour{0xff191719});

    return ed;
}

void CustomLabel::resized() {
    auto ed = getCurrentTextEditor();
    if (ed != nullptr) {
        Rectangle<int> area = getLocalBounds();
        ed->setBounds (area.reduced(15));
    }
}
