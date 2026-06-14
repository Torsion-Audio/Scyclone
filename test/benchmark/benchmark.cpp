#include <benchmark/benchmark.h>
#include <PluginEditor.h>

static void BM_processor(benchmark::State &state)
{
    auto gui = juce::ScopedJuceInitialiser_GUI{};
    for (auto _ : state)
    {
        AudioPluginAudioProcessor processor;
    }
}

BENCHMARK(BM_processor);

static void BM_editor(benchmark::State &state)
{
    auto gui = juce::ScopedJuceInitialiser_GUI{};
    AudioPluginAudioProcessor plugin;
    for (auto _ : state)
    {
        auto editor = plugin.createEditor();
        plugin.editorBeingDeleted(editor);
        delete editor;
    }
}

BENCHMARK(BM_editor);

BENCHMARK_MAIN();
