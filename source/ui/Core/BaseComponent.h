#ifndef SCYCLONE_BASECOMPONENT_H
#define SCYCLONE_BASECOMPONENT_H

#include "JuceHeader.h"
#include "Layout.h"
#include "../LookAndFeel/CustomFontLookAndFeel.h"

class BaseComponent : public juce::Component {
public:
    BaseComponent() { layout.setParent(this); }

    virtual void defineLayout() {}
    virtual void scaleChanged(float /*scale*/) {}

    void resized() override {
        if (layout.size() == 0)
            defineLayout();

        const float scale = CustomFontLookAndFeel::getScale();
        layout.apply(scale);
        scaleChanged(scale);
    }

protected:
    Layout layout;

    static juce::Font getFont(FontType type, float height) { return FontStore::get(type, height); }
};

#endif // SCYCLONE_BASECOMPONENT_H
