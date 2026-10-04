#ifndef SCYCLONE_SCYCLONEMODELCONFIG_H
#define SCYCLONE_SCYCLONEMODELCONFIG_H

#include <anira/anira.h>
#include <string>
#include "OnnxModel.h"

anira::InferenceConfig makeScycloneInferenceConfig(RaveModel model);
anira::InferenceConfig makeScycloneInferenceConfigFromPath(const std::string& modelPath);

/// Checks that @p modelPath is an ONNX model anira can drive with Scyclone's RAVE config,
/// i.e. one input and one output tensor whose shapes are compatible with {1, 1, 2048}.
///
/// A file ORT cannot load at all would also fail inside anira, but this reports why. The shape
/// check matters more: a structurally valid ONNX with the wrong tensor shape loads fine, and the
/// mismatch only surfaces at anira's warm-up Run, which catches it — silently, with
/// ANIRA_WITH_LOGGING off — and lets the model through to produce garbage audio. This restores
/// the shape check the pre-anira InferenceThread did via GetInputTypeInfo().
///
/// @param errorOut set to a human-readable reason when validation fails.
/// @return true if the file can be loaded with the expected shape.
bool validateRaveModelFile(const std::string& modelPath, std::string& errorOut);

#endif // SCYCLONE_SCYCLONEMODELCONFIG_H
