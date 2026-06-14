#pragma once

/// @file BlockStreaming.h
/// @brief Generic block-streaming helpers for audio processor tests.
///
/// Domain-neutral arrange/act utilities for any processor exposing
/// `processBlock(juce::AudioBuffer<float>&)`. Mirrors libsamplerate
/// `streaming_test.c` block iteration patterns.
///
/// Include when driving a processor in fixed-size chunks (pre-roll, stimulus
/// streaming, output collection).
///
/// @namespace torsion::test

#include <algorithm>
#include <vector>
#include <JuceHeader.h>

namespace torsion::test
{

    /// Flushes processor state with @p nBlocks of cleared input (warmup / pre-roll).
    /// @param proc  Processor under test; must be prepared.
    /// @param buf   Reusable mono buffer sized to the processor block.
    /// @param nBlocks Number of silence blocks to process.
    template <typename Processor>
    inline void runSilencePreRollBlocks(Processor &proc, juce::AudioBuffer<float> &buf, int nBlocks)
    {
        buf.clear();
        for (int n = 0; n < nBlocks; ++n)
        {
            (void)proc.processBlock(buf);
        }
    }

    /// Streams @p input through @p proc in @p buf-sized chunks; appends all output samples.
    /// Partial final blocks are zero-padded in the host buffer before each call.
    template <typename Processor>
    inline void streamBufferThroughProcessor(Processor &proc, juce::AudioBuffer<float> &buf,
                                             const float *input, int inputLen, std::vector<float> &output)
    {
        const int blockSize = buf.getNumSamples();
        for (int pos = 0; pos < inputLen; pos += blockSize)
        {
            buf.clear();
            const int copyLen = std::min(blockSize, inputLen - pos);
            for (int i = 0; i < copyLen; ++i)
            {
                buf.setSample(0, i, input[pos + i]);
            }
            juce::AudioBuffer<float> &out = proc.processBlock(buf);
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                output.push_back(out.getSample(0, i));
            }
        }
    }

    /// Pre-roll then stream: convenience for SNR and structural long-run tests.
    /// @param outputReserve Hint for `vector::reserve` (e.g. inputLen * srcRatio).
    /// @return Contiguous output sample stream (latency + stimulus tail included).
    template <typename Processor>
    inline std::vector<float> collectProcessorOutput(Processor &proc, juce::AudioBuffer<float> &buf,
                                                     const float *input, int inputLen, int preRollBlocks,
                                                     size_t outputReserve = 0)
    {
        runSilencePreRollBlocks(proc, buf, preRollBlocks);
        std::vector<float> output;
        if (outputReserve > 0)
        {
            output.reserve(outputReserve);
        }
        streamBufferThroughProcessor(proc, buf, input, inputLen, output);
        return output;
    }

    /// Appends channel 0 of @p buf to @p output (used by chain collectors).
    inline void appendBufferSamples(const juce::AudioBuffer<float> &buf, std::vector<float> &output)
    {
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            output.push_back(buf.getSample(0, i));
        }
    }

} // namespace torsion::test
