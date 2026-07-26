#ifndef SCYCLONE_SCYCLONEMODELCONFIG_H
#define SCYCLONE_SCYCLONEMODELCONFIG_H

#include <anira/anira.h>
#include <string>
#include "OnnxModel.h"

anira::InferenceConfig makeScycloneInferenceConfig(RaveModel model);
anira::InferenceConfig makeScycloneInferenceConfigFromPath(const std::string& modelPath);

#endif // SCYCLONE_SCYCLONEMODELCONFIG_H
