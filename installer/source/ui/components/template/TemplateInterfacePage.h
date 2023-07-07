//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_TEMPLATEINTERFACEPAGE_H
#define GUI_APP_EXAMPLE_TEMPLATEINTERFACEPAGE_H

#include "JuceHeader.h"
#include "../customComponent/CustomButton.h"

class TemplateInterfacePage : public juce::Component, public CustomButton::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void pageFinished(TemplateInterfacePage* reference) = 0;
        virtual void cancelProcess(TemplateInterfacePage* reference) = 0;
    };

    virtual void addListener(Listener* listener) {
        listeners.add(listener);
    }
    virtual void removeListener(Listener* listener) {
        listeners.remove(listener);
    }

protected:
    void initComponent(juce::Component* component) {
        if (auto* label = dynamic_cast<juce::Label*> (component)) {
            label->setJustificationType (juce::Justification::centred);
            label->setFont (juce::Font (16.0f, juce::Font::bold));
            addAndMakeVisible(label);
        } else if (auto* but = dynamic_cast<CustomButton*> (component)) {
            but->addListener(this);
            addAndMakeVisible(but);
        } else if (auto* toggleButton = dynamic_cast<juce::ToggleButton*> (component)) {
            toggleButton->setToggleState(true, juce::NotificationType::dontSendNotification);
            addAndMakeVisible(toggleButton);
        }
    }

    virtual void buttonClicked() override {
        notifyFinished(this);
    }

    void notifyFinished(TemplateInterfacePage* reference) {
        listeners.call([reference](Listener& l) { l.pageFinished(reference); });
    }

    void notifyCancel(TemplateInterfacePage* reference) {
        listeners.call([reference](Listener& l) { l.cancelProcess(reference); });
    }

private:
    juce::ListenerList<Listener> listeners;

};

#endif //GUI_APP_EXAMPLE_TEMPLATEINTERFACEPAGE_H
