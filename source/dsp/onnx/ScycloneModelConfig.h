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
/// The two checks guard different failure modes, both verified against anira 688a396:
///
/// 1. Loadability. anira::Context::create_session() bumps its active-session counter in
///    get_available_session_id() before set_processor() constructs the Ort::Session, and only
///    appends to m_sessions afterwards. A file ORT cannot load at all (corrupt bytes, missing
///    path, unsupported opset) therefore throws out of the constructor with the counter already
///    incremented. Measured: the counter goes 0 -> 1 and never returns while m_sessions stays
///    empty, so release_session()'s `fetch_sub(1) == 1` gate never fires and the shared thread
///    pool and Context singleton leak for the life of the process — in a DAW, once per failed
///    load. Validating up front keeps anira on its happy path.
///
/// 2. Shape. A structurally valid ONNX with the wrong tensor shape loads fine; the mismatch
///    only surfaces at anira's warm-up Run, which catches and logs it. So this check is not
///    about the leak above — it exists to reject a wrong model with a clear message instead of
///    letting it through to produce garbage audio, restoring the shape check the pre-anira
///    InferenceThread did via GetInputTypeInfo().
///
/// @param errorOut set to a human-readable reason when validation fails.
/// @return true if the file can be loaded with the expected shape.
bool validateRaveModelFile(const std::string& modelPath, std::string& errorOut);

#endif // SCYCLONE_SCYCLONEMODELCONFIG_H
