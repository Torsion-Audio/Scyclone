//
// Created by valentin.ackva on 22.06.2023.
//

#ifndef GUI_APP_EXAMPLE_UTILCLASS_H
#define GUI_APP_EXAMPLE_UTILCLASS_H

#include "JuceHeader.h"

class UtilClass {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void processFinished(UtilClass* reference, bool status) = 0;
    };

    virtual void addListener(Listener* listener) {
        listeners.add(listener);
    }
    virtual void removeListener(Listener* listener) {
        listeners.remove(listener);
    }

protected:
    void notifyFinished(UtilClass* reference, bool success) {
        listeners.call([reference, success](Listener& l) { l.processFinished(reference, success); });
    }

private:
    juce::ListenerList<Listener> listeners;

};

#endif //GUI_APP_EXAMPLE_UTILCLASS_H
