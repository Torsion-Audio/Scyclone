//
//  RNBO_PresetList.cpp
//  RNBO
//
//  Created by Samuel Tarakajian on 11/9/20.
//

#if !(defined(RNBO_NOPRESETS) || defined(RNBO_NOJSONPRESETS))

#include "RNBO_PresetList.h"

namespace RNBO {

    PresetList::PresetList(std::string jsonString):
        _parsedPresets()
    {
        convertJSONArrayToPresetList(jsonString, _parsedPresets);
    }
    PresetList::~PresetList() {}

    size_t PresetList::size()
    {
        return _parsedPresets.size();
    }

    UniquePresetPtr PresetList::presetAtIndex(size_t index)
    {
		if (index > this->size()) return nullptr;
        UniquePresetPtr preset = make_unique<Preset>();
        PresetPtr srcPreset = _parsedPresets[index]->preset;
        copyPreset(*srcPreset, *preset);
        return preset;
    }

    std::string PresetList::presetNameAtIndex(size_t index)
    {
		if (index > this->size()) return "";
        std::string name = _parsedPresets[index]->name;
        return name;
    }

    UniquePresetPtr PresetList::presetWithName(std::string name)
    {
        for (size_t i = 0; i < _parsedPresets.size(); i++) {
            std::shared_ptr<NamedPresetEntry> entry = _parsedPresets[i];
            if (entry->name == name) {
                UniquePresetPtr preset = make_unique<Preset>();
                PresetPtr srcPreset = entry->preset;
                copyPreset(*srcPreset, *preset);
                return preset;
            }
        }

        return nullptr;
    }
}

#endif /* RNBO_NOPRESETS */
