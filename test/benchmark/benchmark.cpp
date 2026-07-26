#include <benchmark/benchmark.h>
#include <cmath>
#include <memory>
#include <vector>

#include <PluginEditor.h>

namespace
{
constexpr double kSampleRate = 44100.0;
constexpr int kBlockSize = 512;
constexpr int kPreRollBlocks = 4;

static void BM_reference_cpu(benchmark::State &state)
{
    double x = 1.0;
    for (auto _ : state)
    {
        for (int i = 0; i < 1000000; ++i)
            x = std::fma(x, 1.0000001, 0.0000001);
        benchmark::DoNotOptimize(x);
    }
}

BENCHMARK(BM_reference_cpu)->MinTime(2.0);

static void BM_reference_memory(benchmark::State &state)
{
    constexpr size_t kSize = 256 * 1024;
    std::vector<float> buf(kSize);
    for (auto _ : state)
    {
        for (size_t i = 0; i < kSize; ++i)
            buf[i] = static_cast<float>(i) * 0.001f;
        float sum = 0.0f;
        for (size_t i = 0; i < kSize; i += 17)
            sum += buf[i] * 1.0001f + 0.0001f;
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(BM_reference_memory)->MinTime(2.0);

class ProcessorFixture : public benchmark::Fixture
{
public:
    void SetUp(const benchmark::State &) override
    {
        gui = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
        processor = std::make_unique<AudioPluginAudioProcessor>();
    }

    void TearDown(const benchmark::State &) override
    {
        if (processor != nullptr)
            processor->releaseResources();

        processor.reset();
        gui.reset();
    }

protected:
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> gui;
    std::unique_ptr<AudioPluginAudioProcessor> processor;
};

BENCHMARK_DEFINE_F(ProcessorFixture, BM_processor_prepare)(benchmark::State &state)
{
    for (auto _ : state)
    {
        processor->prepareToPlay(kSampleRate, kBlockSize);
        processor->releaseResources();
    }
}
BENCHMARK_REGISTER_F(ProcessorFixture, BM_processor_prepare)->MinTime(2.0);

class PreparedProcessorFixture : public ProcessorFixture
{
public:
    void SetUp(const benchmark::State &state) override
    {
        ProcessorFixture::SetUp(state);
        processor->prepareToPlay(kSampleRate, kBlockSize);
        buffer.setSize(processor->getTotalNumOutputChannels(), kBlockSize);
        buffer.clear();

        // Warm ONNX inference threads before timing (matches PluginIntegrationTest).
        juce::MidiBuffer midi;
        for (int block = 0; block < kPreRollBlocks; ++block)
            processor->processBlock(buffer, midi);
    }

protected:
    juce::AudioBuffer<float> buffer;
};

BENCHMARK_DEFINE_F(PreparedProcessorFixture, BM_process_block)(benchmark::State &state)
{
    juce::MidiBuffer midi;
    for (auto _ : state)
    {
        processor->processBlock(buffer, midi);
        benchmark::DoNotOptimize(buffer.getArrayOfWritePointers());
    }
}
BENCHMARK_REGISTER_F(PreparedProcessorFixture, BM_process_block)->MinTime(2.0);

BENCHMARK_DEFINE_F(ProcessorFixture, BM_editor)(benchmark::State &state)
{
    for (auto _ : state)
    {
        auto *editor = processor->createEditor();
        processor->editorBeingDeleted(editor);
        delete editor;
    }
}
BENCHMARK_REGISTER_F(ProcessorFixture, BM_editor)->MinTime(2.0);

} // namespace

BENCHMARK_MAIN();
