//Audio Units entrypoint/factory
//defines pulled from JUCE

#ifndef AUSDK_EXPORT
#if __GNUC__
#define AUSDK_EXPORT __attribute__((visibility("default"))) // NOLINT
#else
#warning export?
#endif
#endif

#define JUCE_AU_ENTRY_POINT_NAME RNBO_Plugin_AUFactory

//forward decl
extern "C" void* StaticAUExportFactory(const void * desc);
extern "C" void* JUCE_AU_ENTRY_POINT_NAME (const /*AudioComponentDescription*/ void* inDesc);

AUSDK_EXPORT extern "C" void* JUCE_AU_ENTRY_POINT_NAME (const /*AudioComponentDescription*/ void* inDesc)
{
		return StaticAUExportFactory(inDesc);
}
