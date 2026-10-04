#ifndef SCYCLONE_SCYCLONEMODELCONFIG_H
#define SCYCLONE_SCYCLONEMODELCONFIG_H

#include <anira/anira.h>
#include <cstddef>
#include <string>
#include "OnnxModel.h"

/// Config for one of the embedded models (BinaryData, alive for the whole process).
anira::InferenceConfig makeScycloneInferenceConfig(RaveModel model);

/// Config for a model held in memory. anira's binary ModelData only points at the bytes, so the
/// caller must keep them alive for as long as this config, or any copy of it, can still be used
/// to build a session. User-supplied models must have passed validateRaveModel() first.
anira::InferenceConfig makeScycloneInferenceConfig(const void* modelBytes, size_t modelSize);

/// Checks that the model bytes are an ONNX model anira can drive with Scyclone's RAVE config:
/// one input and one output tensor, both float32, with shapes compatible with {1, 1, 2048}.
///
/// A model ORT cannot load at all would also fail inside anira, but this reports why. The type
/// and shape checks matter more: a structurally valid ONNX with the wrong element type or tensor
/// shape loads fine, and the mismatch only surfaces in anira's Run, which catches it — silently,
/// with ANIRA_WITH_LOGGING off — so the network would be silent or produce garbage audio. This
/// restores the shape check the pre-anira InferenceThread did via GetInputTypeInfo().
///
/// @param errorOut set to a human-readable reason when validation fails.
/// @return true if the model can be loaded with the expected type and shape.
bool validateRaveModel(const void* modelBytes, size_t modelSize, std::string& errorOut);

#endif // SCYCLONE_SCYCLONEMODELCONFIG_H
