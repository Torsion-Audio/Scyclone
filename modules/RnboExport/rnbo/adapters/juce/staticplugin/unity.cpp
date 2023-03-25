#define UNITY_INTERFACE_EXPORT   __attribute__ ((visibility("default")))
#define UNITY_INTERFACE_API

//forward decl
extern "C" int UNITY_INTERFACE_API JuceStatic_UnityGetAudioEffectDefinitions (void*** definitionsPtr);

//entrypoint trampoline to JUCE
extern "C" UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API UnityGetAudioEffectDefinitions (void*** definitionsPtr)
{
	return JuceStatic_UnityGetAudioEffectDefinitions(definitionsPtr);
}

