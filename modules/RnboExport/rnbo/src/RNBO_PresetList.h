//
//  RNBO_PresetList.h
//  RNBO
//
//  Created by Samuel Tarakajian on 11/9/20.
//

#ifndef RNBO_PresetList_h
#define RNBO_PresetList_h

#include "RNBO_Presets.h"

#if !(defined(RNBO_NOPRESETS) || defined(RNBO_NOJSONPRESETS))


namespace RNBO {
    class PresetList {
    public:
        PresetList(std::string jsonString);
        ~PresetList();

        size_t size();

        UniquePresetPtr presetAtIndex(size_t index);

        std::string presetNameAtIndex(size_t index);

        UniquePresetPtr presetWithName(std::string name);

    private:
        std::vector<std::shared_ptr<NamedPresetEntry>> _parsedPresets;
    };
}

#endif /* RNBO_NOPRESETS */

#endif /* RNBO_PresetList_h */
