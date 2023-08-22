#include <gtest/gtest.h>
#include <benchmark/benchmark.h>
#include <complex>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <time.h>
#include <PluginEditor.h>


static void BM_processor(benchmark::State& state) {
  // Perform setup here
    auto gui = juce::ScopedJuceInitialiser_GUI {};
  for (auto _ : state) {
    // This code gets timed
    AudioPluginAudioProcessor processor;
  }
}
// Register the function as a benchmark
BENCHMARK(BM_processor);

static void BM_editor(benchmark::State& state) {
  // Perform setup here
    auto gui = juce::ScopedJuceInitialiser_GUI {};
    AudioPluginAudioProcessor plugin;
  for (auto _ : state) {
    // This code gets timed
    auto editor = plugin.createEditor();
    plugin.editorBeingDeleted (editor);
    delete editor;
  }
}
// Register the function as a benchmark
BENCHMARK(BM_editor);

TEST(test, Benchmark){
    benchmark::RunSpecifiedBenchmarks();
}