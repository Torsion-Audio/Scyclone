// rnbo_lib.cpp includes all the source files for RNBO
// making it easy to include RNBO as source into a project.
// Simply add this one file to your compilation.

#include "RNBO.h"

#include "src/RNBO_PlatformInterfaceImpl.cpp" // comes first to ensure that platform is available
#include "src/RNBO_CoreObject.cpp"
#include "src/RNBO_DataBuffer.cpp"
#include "src/RNBO_DataRefList.cpp"
#include "src/RNBO_DynamicPatcherFactory.cpp"
#include "src/RNBO_Engine.cpp"
#include "src/RNBO_ExternalLoader.cpp"
#include "src/RNBO_FileChangeWatcher.cpp"
#include "src/RNBO_Logger.cpp"
#include "src/RNBO_MaxClang.cpp"
#include "src/RNBO_ParameterInterfaceAsync.cpp"
#include "src/RNBO_ParameterInterfaceSync.cpp"
#include "src/RNBO_PatcherFactory.cpp"
#include "src/RNBO_PatcherState.cpp"
#include "src/RNBO_PresetList.cpp"
#include "src/RNBO_List.cpp"
#include "src/RNBO_UnitTests.cpp"
