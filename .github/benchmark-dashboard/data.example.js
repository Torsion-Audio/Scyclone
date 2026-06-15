// Example data for local preview: python -m http.server 8080 in this directory.
window.BENCHMARK_DATA = {
  lastUpdate: Date.now(),
  repoUrl: "https://github.com/Torsion-Audio/Scyclone",
  entries: {
    "Scyclone macOS": [
      {
        commit: {
          id: "abc1111111111111111111111111111111111111111",
          message: "feat(benchmark): example run",
          timestamp: "2026-06-01T12:00:00Z",
          url: "https://github.com/Torsion-Audio/Scyclone/commit/abc1111",
          committer: { username: "example" },
        },
        date: Date.now() - 6 * 86400000,
        tool: "googlecpp",
        benches: [
          { name: "BM_reference_cpu/min_time:2.000_mean", value: 1.05e6, range: "± 1%", unit: "ns/iter" },
          { name: "BM_reference_memory/min_time:2.000_mean", value: 8.0e5, range: "± 1%", unit: "ns/iter" },
          { name: "BM_processor_prepare/min_time:2.000_mean", value: 2.5e7, range: "± 2%", unit: "ns/iter" },
          { name: "BM_process_block/min_time:2.000_mean", value: 1.2e7, range: "± 2%", unit: "ns/iter" },
          { name: "BM_editor/min_time:2.000_mean", value: 9.0e7, range: "± 2%", unit: "ns/iter" },
          { name: "BM_compile_benchmark_target/min_time:2.000_mean", value: 1.85e11, range: "", unit: "ns/iter" },
        ],
      },
      {
        commit: {
          id: "def2222222222222222222222222222222222222222",
          message: "ci(benchmark): second sample point",
          timestamp: "2026-06-08T12:00:00Z",
          url: "https://github.com/Torsion-Audio/Scyclone/commit/def2222",
          committer: { username: "example" },
        },
        date: Date.now(),
        tool: "googlecpp",
        benches: [
          { name: "BM_reference_cpu/min_time:2.000_mean", value: 1.07e6, range: "± 1%", unit: "ns/iter" },
          { name: "BM_reference_memory/min_time:2.000_mean", value: 8.1e5, range: "± 1%", unit: "ns/iter" },
          { name: "BM_processor_prepare/min_time:2.000_mean", value: 2.9e7, range: "± 2%", unit: "ns/iter" },
          { name: "BM_process_block/min_time:2.000_mean", value: 1.22e7, range: "± 2%", unit: "ns/iter" },
          { name: "BM_editor/min_time:2.000_mean", value: 9.1e7, range: "± 2%", unit: "ns/iter" },
          { name: "BM_compile_benchmark_target/min_time:2.000_mean", value: 1.92e11, range: "", unit: "ns/iter" },
        ],
      },
    ],
  },
};
