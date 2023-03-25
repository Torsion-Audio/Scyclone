//grabed/edited from the VST3 SDK
//https://github.com/steinbergmedia/vst3_pluginterfaces/blob/b8566ef3b2a0cba60a96e3ef2001224c865c8b36/base/fplatform.h
#if defined (_WIN32)
	#define PLUGIN_API __stdcall
	#define SMTG_EXPORT_SYMBOL __declspec (dllexport)
#elif __gnu_linux__ || __linux__
	#define PLUGIN_API
	#define SMTG_EXPORT_SYMBOL __attribute__ ((visibility ("default")))
#elif __APPLE__
	#define PLUGIN_API
	#define SMTG_EXPORT_SYMBOL __attribute__ ((visibility ("default")))
#else
	#pragma error unknown platform
#endif

//forward decl
class IPluginFactory;
extern "C" IPluginFactory* PLUGIN_API JuceStatic_GetPluginFactory();

//VST3 entrypoint
extern "C" SMTG_EXPORT_SYMBOL IPluginFactory* PLUGIN_API GetPluginFactory()
{
	return JuceStatic_GetPluginFactory();
}

#include <inttypes.h>
extern "C" {
  uint32_t JuceStatic_Plugin_VSTNumMidiInputs() {
    return PLUGIN_NUM_MIDI_INPUTS;
  }
  uint32_t JuceStatic_Plugin_VSTNumMidiOutputs() {
    return PLUGIN_NUM_MIDI_OUTPUTS;
  }
}
