//forward decl
struct _dictionary;
typedef _dictionary t_dictionary;

namespace RNBO {
	class PatcherState;
	class CoreObject;
	using Preset = PatcherState;
}

namespace MaxPresetAdapter {
	void toDict(const RNBO::Preset& preset, t_dictionary* presetDict);
	void fromDict(t_dictionary* presetDict, RNBO::Preset& preset);
	//XXX Note, this will block while waiting for the preset
	void getObjectPreset(RNBO::CoreObject& o, t_dictionary *presetDict, bool dspIsOn);
	void setObjectPreset(RNBO::CoreObject& o, t_dictionary *presetDict);
}
