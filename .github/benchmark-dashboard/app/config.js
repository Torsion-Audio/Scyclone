export const chartPalette = [
  '#EB1E79', '#00FFE6', '#6094EA', '#F02FC2', '#18A5A7',
  '#C82471', '#CCFFAA', '#004E92',
];

export const chartTheme = {
  grid: 'rgba(255, 255, 255, 0.04)',
  tick: 'rgba(255, 255, 255, 0.42)',
  tickMuted: 'rgba(255, 255, 255, 0.22)',
  label: 'rgba(255, 255, 255, 0.45)',
  tooltipBg: 'rgba(28, 28, 30, 0.92)',
  tooltipTitle: '#f5f5f7',
  tooltipBody: 'rgba(255, 255, 255, 0.78)',
  tooltipFooter: 'rgba(255, 255, 255, 0.42)',
  tooltipBorder: 'rgba(255, 255, 255, 0.1)',
};

export const benchMeta = {
  reference_cpu: {
    label: 'Reference CPU',
    summary: 'Synthetic FMA loop, CPU baseline.',
    tooltip: 'Synthetic CPU baseline on the CI runner.\n\n'
      + 'Each iteration\n'
      + '1M fused-multiply-add operations\n\n'
      + 'Not Scyclone code — use to spot runner drift.',
    kind: 'sentinel',
  },
  reference_memory: {
    label: 'Reference Memory',
    summary: 'Synthetic ~1 MiB fill/read, memory baseline.',
    tooltip: 'Synthetic memory baseline on the CI runner.\n\n'
      + 'Each iteration\n'
      + 'Fill ~1 MiB, then strided reads\n\n'
      + 'Not Scyclone code — use to spot runner drift.',
    kind: 'sentinel',
  },
  processor_prepare: {
    label: 'Prepare',
    summary: 'Host prepare/release on an already-constructed processor.',
    tooltip: 'Host prepare/release on a ready-built processor.\n\n'
      + 'Fixture\n'
      + 'Processor created once at setup\n\n'
      + 'Each iteration\n'
      + 'prepareToPlay(44100, 512) → releaseResources()\n\n'
      + 'ONNX models load at setup, not per iteration.',
    kind: 'app',
  },
  processor: {
    label: 'Prepare',
    summary: 'Host prepare/release on an already-constructed processor.',
    tooltip: 'Legacy name: BM_processor\n'
      + 'Same measurement as Prepare.\n\n'
      + 'Fixture\n'
      + 'Processor created once at setup\n\n'
      + 'Each iteration\n'
      + 'prepareToPlay(44100, 512) → releaseResources()',
    kind: 'app',
  },
  process_block: {
    label: 'Process',
    summary: 'Stereo 512-sample audio callback, the real-time DSP path.',
    tooltip: 'Real-time stereo audio callback — the DSP hot path.\n\n'
      + 'Fixture\n'
      + 'Processor prepared at 44.1 kHz / 512 samples\n\n'
      + 'Each iteration\n'
      + 'processBlock() on a cleared stereo buffer',
    kind: 'app',
  },
  editor: {
    label: 'Editor',
    summary: 'UI open/close cost; catches heavier components or layout regressions.',
    tooltip: 'Plugin editor open/close cost.\n\n'
      + 'Fixture\n'
      + 'Processor created once\n\n'
      + 'Each iteration\n'
      + 'Create and destroy the full editor\n'
      + '(JUCE UI + OpenGL background)\n\n'
      + 'Headless CI — no physical display attached.',
    kind: 'app',
  },
  compile_benchmark_target: {
    label: 'Compile Time',
    kpiLabel: 'Compile',
    summary: 'Wall-clock Release build of the Benchmark executable and its dependencies.',
    tooltip: 'Wall-clock Release build time.\n\n'
      + 'Measured\n'
      + 'Once per CI run, after a fresh configure\n\n'
      + 'Target\n'
      + 'Benchmark executable and its dependencies\n\n'
      + 'FetchContent deps may be cached; object files are not reused between runs.',
    kind: 'build',
  },
};

export const appBenchOrder = ['processor_prepare', 'process_block', 'editor'];
export const buildBenchOrder = ['compile_benchmark_target'];
export const sentinelBenchOrder = ['reference_cpu', 'reference_memory'];

export const chartCommitTickRotation = 45;
export const chartCommitTickPadding = 48;
export const chartCommitTickPlaceholder = '0000000';

/** Fixed plot height for stacked inline KPI rows (maintainAspectRatio off). */
export const stackedChartHeightPx = 168;
/** Fixed y-axis gutter so commit columns align across stacked charts. */
export const stackedYAxisWidthPx = 56;
/** Reserved x-axis gutter (rotated commit labels) on every stacked row for alignment. */
export const stackedXAxisHeightPx = 52;

export const deltaFlatThresholdPct = 0.05;
export const deltaNoiseBandPct = 3;
export const deltaAlertThresholdPct = 5;
