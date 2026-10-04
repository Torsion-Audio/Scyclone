#ifndef SCYCLONE_TEST_WETLAGMEASUREMENT_H
#define SCYCLONE_TEST_WETLAGMEASUREMENT_H

// Measures how far a (possibly generative) model delays transients: the input is a train of short
// decaying noise bursts, the output is searched for the response to each one, and the lag is the
// distance between input and output onsets. RAVE does not reproduce its input, so this measures
// *when* energy comes out, not what comes out. Shared by the DISABLED OnnxWetLagProbe and the
// pinned wet-alignment tests; WetLagEstimatorTest checks the estimator itself on synthetic data.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <random>
#include <vector>

namespace scyclone::test::wetlag
{

struct StimulusSpec
{
    int hop = 2048;          ///< model hop; bursts are placed at evenly spread phases within it
    int spacing = 16384;     ///< samples between bursts; a multiple of hop, > 2x the largest lag
    int numPhases = 8;       ///< distinct burst start phases within the hop
    int burstsPerPhase = 4;
    int leadInHops = 4;      ///< silence before the first burst
    int burstLength = 256;
    float tau = 64.0f;       ///< decay of the burst envelope, in samples
    float peak = 0.8f;
    std::uint32_t seed = 1234;
};

struct Stimulus
{
    std::vector<float> samples;
    std::vector<int> burstStarts; ///< sample index where each burst begins
    std::vector<int> burstPhases; ///< burst start modulo the hop
};

inline Stimulus makeBurstStimulus(const StimulusSpec& spec = {})
{
    Stimulus stimulus;
    const int numBursts = spec.numPhases * spec.burstsPerPhase;
    const int firstSlot = spec.leadInHops * spec.hop;
    stimulus.samples.assign(static_cast<size_t>(firstSlot + (numBursts + 1) * spec.spacing), 0.0f);

    std::mt19937 rng(spec.seed);
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    const int phaseStep = spec.hop / spec.numPhases;

    for (int k = 0; k < numBursts; ++k)
    {
        // Slots start on hop boundaries, so the start's position within the hop is exactly `phase`.
        const int phase = (k % spec.numPhases) * phaseStep + phaseStep / 2;
        const int start = firstSlot + k * spec.spacing + phase;
        for (int n = 0; n < spec.burstLength; ++n)
            stimulus.samples[static_cast<size_t>(start + n)] =
                spec.peak * noise(rng) * std::exp(-static_cast<float>(n) / spec.tau);
        stimulus.burstStarts.push_back(start);
        stimulus.burstPhases.push_back(phase);
    }
    return stimulus;
}

struct EnvelopeSpec
{
    int frame = 32; ///< samples per energy frame
    int step = 8;   ///< samples between frames
};

/// Mean-square energy of the DC-blocked signal over `frame` samples, every `step` samples.
/// Frame f covers samples [f * step, f * step + frame).
inline std::vector<float> energyEnvelope(const std::vector<float>& signal, EnvelopeSpec spec = {})
{
    std::vector<float> highPassed(signal.size());
    float previousIn = 0.0f;
    float previousOut = 0.0f;
    for (size_t i = 0; i < signal.size(); ++i)
    {
        previousOut = signal[i] - previousIn + 0.995f * previousOut;
        previousIn = signal[i];
        highPassed[i] = previousOut;
    }

    if (signal.size() < static_cast<size_t>(spec.frame))
        return {};

    const size_t frames = (signal.size() - static_cast<size_t>(spec.frame)) / static_cast<size_t>(spec.step) + 1;
    std::vector<float> envelope(frames);
    for (size_t f = 0; f < frames; ++f)
    {
        double sum = 0.0;
        const size_t begin = f * static_cast<size_t>(spec.step);
        for (size_t i = begin; i < begin + static_cast<size_t>(spec.frame); ++i)
            sum += static_cast<double>(highPassed[i]) * highPassed[i];
        envelope[f] = static_cast<float>(sum / spec.frame);
    }
    return envelope;
}

inline double median(std::vector<double> values)
{
    if (values.empty())
        return std::nan("");
    const auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
    std::nth_element(values.begin(), middle, values.end());
    if (values.size() % 2 == 1)
        return *middle;
    const double upper = *middle;
    const double lower = *std::max_element(values.begin(), middle);
    return 0.5 * (lower + upper);
}

struct LagSearch
{
    int minLag = 0;      ///< earliest plausible response, in samples after the input onset
    int maxLag = 12288;  ///< latest plausible response
    int floorSpan = 4096; ///< samples before the search window used for the local noise floor
    double minPeakToFloor = 10.0; ///< energy ratio (10 dB) a response must exceed
    double onsetFraction = 0.2;   ///< onset = first crossing of floor + fraction * (peak - floor)
};

struct LagEstimate
{
    int bursts = 0;
    int responses = 0;
    double responseRate = 0.0;
    double medianLag = std::nan("");
    double medianAbsoluteDeviation = std::nan("");
    std::map<int, double> phaseMedians;     ///< burst phase -> median lag of its responses
    double maxPhaseDeviation = std::nan(""); ///< largest |phase median - overall median|
    int crossCorrelationLag = -1;            ///< lag maximising log-envelope cross-correlation

    /// Median of (lag + burst phase): where a response starts relative to the start of the hop the
    /// burst fell into. A model that only resolves time to the hop emits every burst's response at
    /// the same point of its output hop, so this is stable even when medianLag varies with phase.
    double medianHopAlignedOnset = std::nan("");
};

namespace detail
{
    /// Onset frame of the first threshold crossing in [begin, end), or -1 if there is no response.
    inline int findOnsetFrame(const std::vector<float>& envelope, int begin, int end,
                              int floorBegin, const LagSearch& search)
    {
        begin = std::max(begin, 0);
        end = std::min(end, static_cast<int>(envelope.size()));
        floorBegin = std::max(floorBegin, 0);
        if (begin >= end)
            return -1;

        std::vector<double> floorValues(envelope.begin() + floorBegin, envelope.begin() + std::max(floorBegin, begin));
        const double floor = std::max(floorValues.empty() ? 0.0 : median(floorValues), 1e-12);

        const auto peakIt = std::max_element(envelope.begin() + begin, envelope.begin() + end);
        const double peak = *peakIt;
        if (peak < search.minPeakToFloor * floor)
            return -1;

        const double threshold = floor + search.onsetFraction * (peak - floor);
        for (int f = begin; f < end; ++f)
            if (envelope[static_cast<size_t>(f)] >= threshold)
                return f;
        return -1;
    }

    inline int crossCorrelationLag(const std::vector<float>& inputEnvelope,
                                   const std::vector<float>& outputEnvelope,
                                   int minLagFrames, int maxLagFrames)
    {
        auto logNormalised = [](const std::vector<float>& envelope) {
            std::vector<double> values(envelope.size());
            double mean = 0.0;
            for (size_t i = 0; i < envelope.size(); ++i)
            {
                values[i] = std::log(static_cast<double>(envelope[i]) + 1e-10);
                mean += values[i];
            }
            mean /= std::max<size_t>(values.size(), 1);
            for (auto& v : values)
                v -= mean;
            return values;
        };

        const auto in = logNormalised(inputEnvelope);
        const auto out = logNormalised(outputEnvelope);
        double best = -1e300;
        int bestLag = -1;
        for (int lag = std::max(minLagFrames, 0); lag <= maxLagFrames; ++lag)
        {
            double sum = 0.0;
            const size_t count = std::min(in.size(), out.size() > static_cast<size_t>(lag) ? out.size() - static_cast<size_t>(lag) : 0);
            for (size_t i = 0; i < count; ++i)
                sum += in[i] * out[i + static_cast<size_t>(lag)];
            if (sum > best)
            {
                best = sum;
                bestLag = lag;
            }
        }
        return bestLag;
    }
} // namespace detail

/// Lag of the response to each burst of @p stimulus in @p output (same sample clock). Input and
/// output onsets are found with the same estimator, so its own bias cancels in the difference.
inline LagEstimate estimateLag(const Stimulus& stimulus, const std::vector<float>& output,
                               const LagSearch& search = {}, EnvelopeSpec envelopeSpec = {})
{
    LagEstimate estimate;
    const auto inputEnvelope = energyEnvelope(stimulus.samples, envelopeSpec);
    const auto outputEnvelope = energyEnvelope(output, envelopeSpec);
    const int step = envelopeSpec.step;

    std::vector<double> lags;
    std::vector<double> hopAlignedOnsets;
    std::map<int, std::vector<double>> lagsByPhase;

    for (size_t k = 0; k < stimulus.burstStarts.size(); ++k)
    {
        const int start = stimulus.burstStarts[k];
        ++estimate.bursts;

        // Input onset: search a short window around the known burst start.
        const int inputOnsetFrame = detail::findOnsetFrame(
            inputEnvelope, (start - 128) / step, (start + 256) / step, (start - 2048) / step, search);
        if (inputOnsetFrame < 0)
            continue;
        const int inputOnset = inputOnsetFrame * step;

        const int windowBegin = (inputOnset + search.minLag) / step;
        const int windowEnd = (inputOnset + search.maxLag) / step;
        const int outputOnsetFrame = detail::findOnsetFrame(
            outputEnvelope, windowBegin, windowEnd, windowBegin - search.floorSpan / step, search);
        if (outputOnsetFrame < 0)
            continue;

        const double lag = static_cast<double>(outputOnsetFrame * step - inputOnset);
        lags.push_back(lag);
        hopAlignedOnsets.push_back(lag + stimulus.burstPhases[k]);
        lagsByPhase[stimulus.burstPhases[k]].push_back(lag);
        ++estimate.responses;
    }

    estimate.responseRate = estimate.bursts > 0
        ? static_cast<double>(estimate.responses) / estimate.bursts : 0.0;
    estimate.medianLag = median(lags);
    estimate.medianHopAlignedOnset = median(hopAlignedOnsets);

    if (!lags.empty())
    {
        std::vector<double> deviations;
        for (const double lag : lags)
            deviations.push_back(std::abs(lag - estimate.medianLag));
        estimate.medianAbsoluteDeviation = median(deviations);

        double maxDeviation = 0.0;
        for (const auto& [phase, phaseLags] : lagsByPhase)
        {
            estimate.phaseMedians[phase] = median(phaseLags);
            maxDeviation = std::max(maxDeviation, std::abs(estimate.phaseMedians[phase] - estimate.medianLag));
        }
        estimate.maxPhaseDeviation = maxDeviation;
    }

    estimate.crossCorrelationLag = step * detail::crossCorrelationLag(
        inputEnvelope, outputEnvelope, search.minLag / step, search.maxLag / step);
    return estimate;
}

} // namespace scyclone::test::wetlag

#endif // SCYCLONE_TEST_WETLAGMEASUREMENT_H
