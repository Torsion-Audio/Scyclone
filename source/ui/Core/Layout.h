#ifndef SCYCLONE_LAYOUT_H
#define SCYCLONE_LAYOUT_H

#include "JuceHeader.h"
#include "FontStore.h"

struct LayoutItem {
    juce::Component& component;
    juce::Rectangle<float> referenceBounds;
};

struct LayoutFontItem {
    juce::Label& label;
    juce::Font referenceFont;
};

class Layout {
public:
    void setParent(juce::Component* newParent) noexcept { parent = newParent; }

    void add(juce::Component& component, float x, float y, float width, float height) {
        items.push_back({component, {x, y, width, height}});

        if (parent != nullptr)
            parent->addAndMakeVisible(component);
    }

    void add(juce::Label& label, float x, float y, float width, float height,
             FontType fontType, float referenceFontHeight) {
        add(static_cast<juce::Component&>(label), x, y, width, height);
        fontItems.push_back({label, FontStore::get(fontType, referenceFontHeight)});
    }

    void apply(float scale) const {
        for (auto& item : items)
            item.component.setBounds((item.referenceBounds * scale).toNearestInt());

        for (auto& item : fontItems)
            item.label.setFont(item.referenceFont.withHeight(item.referenceFont.getHeight() * scale));
    }

    int size() const noexcept { return static_cast<int>(items.size()); }

    void clear() noexcept {
        items.clear();
        fontItems.clear();
    }

private:
    juce::Component* parent{nullptr};
    std::vector<LayoutItem> items;
    std::vector<LayoutFontItem> fontItems;
};

#endif // SCYCLONE_LAYOUT_H
