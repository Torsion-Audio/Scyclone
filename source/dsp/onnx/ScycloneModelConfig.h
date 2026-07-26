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
/// This must be called before building an InferenceHandler for a user-supplied file.
/// anira::Context::create_session() increments its active-session counter before constructing
/// the ORT session but only registers the session afterwards, so a throwing model load leaks a
/// half-registered session and corrupts the shared Context (crash at teardown). Validating up
/// front keeps anira on its happy path. It also restores the shape check that the pre-anira
/// InferenceThread did via GetInputTypeInfo().
///
/// @param errorOut set to a human-readable reason when validation fails.
/// @return true if the file can be loaded with the expected shape.
bool validateRaveModelFile(const std::string& modelPath, std::string& errorOut);

#endif // SCYCLONE_SCYCLONEMODELCONFIG_H
