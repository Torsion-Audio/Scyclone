#ifndef SCYCLONE_FONTSTORE_H
#define SCYCLONE_FONTSTORE_H

#include "JuceHeader.h"
#include "../LookAndFeel/CustomFontLookAndFeel.h"

enum class FontType {
    regular,
    medium,
    bold
};

class FontStore {
public:
    static juce::Font get(FontType type, float referenceHeight) {
        switch (type) {
            case FontType::medium: return CustomFontLookAndFeel::getCustomFontMedium().withHeight(referenceHeight);
            case FontType::bold: return CustomFontLookAndFeel::getCustomFontBold().withHeight(referenceHeight);
            case FontType::regular: break;
        }

        return CustomFontLookAndFeel::getCustomFont().withHeight(referenceHeight);
    }
};

#endif // SCYCLONE_FONTSTORE_H
