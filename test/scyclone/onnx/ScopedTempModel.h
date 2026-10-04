#ifndef SCYCLONE_TEST_SCOPEDTEMPMODEL_H
#define SCYCLONE_TEST_SCOPEDTEMPMODEL_H

#include "JuceHeader.h"
#include "BinaryData.h"

/// Writes a copy of the embedded FunkDrum model to a uniquely named temp file and deletes it on
/// scope exit, so concurrent test runs on one machine never share or delete each other's file.
class ScopedTempModel
{
public:
    explicit ScopedTempModel(const juce::String& stem = "scyclone_test_model",
                             const juce::String& suffix = ".ort")
        : file(juce::File::getSpecialLocation(juce::File::tempDirectory)
                   .getNonexistentChildFile(stem, suffix, false))
    {
        juce::FileOutputStream stream(file);
        if (stream.openedOk())
            stream.write(BinaryData::funk_drums_ort, BinaryData::funk_drums_ortSize);
    }

    ~ScopedTempModel() { file.deleteFile(); }

    ScopedTempModel(const ScopedTempModel&) = delete;
    ScopedTempModel& operator=(const ScopedTempModel&) = delete;

    const juce::File& get() const { return file; }

private:
    juce::File file;
};

#endif // SCYCLONE_TEST_SCOPEDTEMPMODEL_H
