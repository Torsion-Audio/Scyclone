//
//  RNBO_Presets.h
//

#ifndef _RNBO_Presets_H_
#define _RNBO_Presets_H_

#ifdef RNBO_NOSTDLIB
#define RNBO_NOPRESETS
#endif

#ifdef RNBO_NOPRESETS

namespace RNBO {
	using Preset = PatcherState;
	using PresetCallback = void(*);
	using UniquePresetPtr = void*;
}

#else

#include "RNBO_PatcherState.h"

RNBO_PUSH_DISABLE_WARNINGS
#include "3rdparty/json/json.hpp"
RNBO_POP_DISABLE_WARNINGS

#include "RNBO_Utils.h"

/**
 * @file RNBO_Presets.h
 * @brief RNBO_Presets
 */


namespace RNBO {

	using PresetMap = StateMap;

    /**
     * @var typedef RNBO::PatcherState RNBO::Preset
     * @brief a preset is a saved patcher state
     */
	using Preset = PatcherState;

    /**
     * @var typedef std::shared_ptr<Preset> RNBO::PresetPtr
     * @brief an shared pointer to a Preset
     *
     * A reference-counted pointer to a Preset
     */
	using PresetPtr = std::shared_ptr<Preset>;

    /**
     * @var typedef std::function<void(std::shared_ptr<const Preset>)> RNBO::PresetCallback
     * @brief a function to execute when a preset is made available
     */
	using PresetCallback = std::function<void(std::shared_ptr<const Preset>)>;

    /**
     * @var typedef std::shared_ptr<const Preset> RNBO::ConstPresetPtr
     * @brief an shared pointer to a const Preset
     *
     * Used for read-only access to a Preset
     */
	using ConstPresetPtr = std::shared_ptr<const Preset>;

	/**
	 * @brief Associates a PresetPtr and its name
	 */
    struct NamedPresetEntry {
        std::string name;
        PresetPtr preset;
    };

#ifdef RNBO_NOSTDLIB
	using UniquePresetPtr = UniquePtr<Preset>;
#else
    /**
     * @var typedef std::unique_ptr<Preset> RNBO::UniquePresetPtr
     * @brief a unique pointer to a Preset
     *
     * An owning pointer to a Preset
     */
	using UniquePresetPtr = std::unique_ptr<Preset>;
#endif // RNBO_NOSTDLIB

#ifndef RNBO_NOJSONPRESETS

	using Json = nlohmann::json;

	static Json convertPresetToJSONObj(const Preset& preset) {
		Json json;
		for (auto const& entry : preset) {
			const char *key = entry.first.c_str();
			auto type = entry.second.getType();
			switch (type) {
				case ValueHolder::NUMBER: {
					number value = (number)entry.second;
					json[key] = value;
					break;
				}
				case ValueHolder::LIST: {
					Json j;
					const list& value = entry.second;
					for (size_t i = 0; i < value.length; i++) {
						j.push_back(value[i]);
					}
					json[key] = j;
					break;
				}
				case ValueHolder::STRING: {
					const char * str = entry.second;
					json[key] = str;
					break;
				}
				case ValueHolder::SUBSTATE: {
					const Preset& subPreset = entry.second;
					json[key] = convertPresetToJSONObj(subPreset);
					break;
				}
				case ValueHolder::SUBSTATEMAP: {
					Index size = entry.second.getSubStateMapSize();
					if (size) {
						Json j;
						for (Index i = 0; i < size; i++) {
							const Preset& subPreset = entry.second[i];
							j.push_back(convertPresetToJSONObj(subPreset));
						}
						json[key] = j;
					}
					break;
				}
				case ValueHolder::INDEX: {
					Index value = (Index)entry.second;
					json[key] = value;
					break;
				}
				case ValueHolder::NONE:
				case ValueHolder::EXTERNAL:
				case ValueHolder::EVENTTARGET:
				case ValueHolder::DATAREF:
				case ValueHolder::MULTIREF:
				case ValueHolder::SIGNAL:
				case ValueHolder::BOOLEAN:
				case ValueHolder::INTVALUE:
				default:
					// we do only support numbers, lists and substates
					RNBO_ASSERT(false);
			}
		}

		return json;
	}

	ATTRIBUTE_UNUSED
	static std::string convertPresetToJSON(const Preset& preset) {
		return convertPresetToJSONObj(preset).dump();
	}

	static void convertJSONObjToPreset(Json& json, Preset& preset) {
		for (Json::iterator it = json.begin(); it != json.end(); it++) {
			const char* key = it.key().c_str();
			if (it->is_number()) {
				number value = it.value();
				preset[key] = value;
			}
			else if (it->is_string()) {
				std::string value = it.value();
				preset[key] = value.c_str();
			}
			else if (it->is_array()) {
				Json& j = *it;
				if (j.size() > 0) {
					if (j[0].is_number()) {
						list value;
						for (Index i = 0; i < j.size(); i++) {
							value.push(j[i]);
						}
						preset[key] = value;
					}
					else if (j[0].is_object()) {
						for (Index i = 0; i < j.size(); i++) {
							Preset& subPreset = preset[key][i];
							convertJSONObjToPreset(j[i], subPreset);
						}
					}
				}
			}
			else if (it->is_object()) {
				Json& j = *it;
				Preset& subPreset = preset.getSubState(key);
				convertJSONObjToPreset(j, subPreset);
			}
		}
	}

    ATTRIBUTE_UNUSED
    static void convertJSONArrayToPresetList(std::string jsonString, std::vector<std::shared_ptr<NamedPresetEntry>>& presetList) {
		Json json = Json::parse(jsonString);
        for (Json::iterator it = json.begin(); it != json.end(); it++) {
            if (it->is_object()) {
                Json& j = *it;
                std::shared_ptr<NamedPresetEntry> entry(new NamedPresetEntry);
                std::string name = j["name"];
                Json presetPayload = j["preset"];
                entry->name = name;
                PresetPtr preset = std::make_shared<Preset>();
                convertJSONObjToPreset(presetPayload, *preset);
                entry->preset = preset;
                presetList.push_back(entry);
            }
        }
    }

	ATTRIBUTE_UNUSED
	static UniquePresetPtr convertJSONToPreset(std::string jsonString) {
		UniquePresetPtr preset = make_unique<Preset>();
		Json json = Json::parse(jsonString);
		convertJSONObjToPreset(json, *preset);
		return preset;
	}

	ATTRIBUTE_UNUSED
	static void copyPreset(const Preset& src, Preset &dst)
	{
			for (auto const& entry : src) {
					const char *key = entry.first.c_str();
					auto type = entry.second.getType();
					switch (type) {
						case ValueHolder::NUMBER: {
							number value = (number)entry.second;
							dst[key] = value;
							break;
						}
						case ValueHolder::INDEX: {
							Index value = (Index)entry.second;
							dst[key] = value;
							break;
						}
						case ValueHolder::LIST: {
							Json j;
							const list& srclist = entry.second;
							list dstlist;
							for (Index i = 0; i < srclist.length; i++) {
									dstlist.push(srclist[i]);
							}
							dst[key] = dstlist;
							break;
						}
						case ValueHolder::SUBSTATE: {
							const Preset& preset = entry.second;
							copyPreset(preset, dst[key]);
							break;
						}
						case ValueHolder::SUBSTATEMAP: {
							Index size = entry.second.getSubStateMapSize();
							if (size) {
									for (Index i = 0; i < size; i++) {
											const Preset& preset = entry.second[i];
											Preset& dstSubPreset = dst[key][i];
											copyPreset(preset, dstSubPreset);
									}
							}
							break;
						}
						case ValueHolder::NONE:
						case ValueHolder::EXTERNAL:
						case ValueHolder::EVENTTARGET:
						case ValueHolder::DATAREF:
						case ValueHolder::MULTIREF:
						case ValueHolder::SIGNAL:
						case ValueHolder::STRING:
						case ValueHolder::BOOLEAN:
						case ValueHolder::INTVALUE:
						default:
							// we do only support numbers, lists and substates
							RNBO_ASSERT(false);
					}
			}
	}

#endif // RNBO_NOJSONPRESETS

} // namespace RNBO

#endif // RNBO_NOPRESETS

#endif  // _RNBO_Presets_H_
