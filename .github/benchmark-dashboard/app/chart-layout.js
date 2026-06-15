import {
  chartCommitTickPlaceholder,
  chartCommitTickRotation,
  chartTheme,
} from './config.js';

export function chartFillColor(hex, alpha = '14') {
  return hex.length === 7 ? hex + alpha : hex;
}

export function chartAspectRatio() {
  const w = window.innerWidth;
  if (w <= 480) return 1.25;
  if (w <= 720) return 1.55;
  return 1.85;
}

export function chartCommitTickFontSize() {
  if (window.innerWidth <= 480) return 9;
  if (window.innerWidth <= 720) return 10;
  return 11;
}

export function configureCommitAxisTicks(chart, labels, emphasized) {
  const xAxis = chart.options.scales.xAxes[0];

  xAxis.ticks.autoSkip = false;
  xAxis.ticks.maxRotation = chartCommitTickRotation;
  xAxis.ticks.minRotation = chartCommitTickRotation;
  xAxis.ticks.fontSize = chartCommitTickFontSize();
  xAxis.ticks.fontFamily = 'Inter';
  xAxis.ticks.fontColor = emphasized ? chartTheme.tick : chartTheme.tickMuted;
  xAxis.ticks.padding = 6;
  xAxis.ticks.callback = function (_value, index) {
    return labels[index] || chartCommitTickPlaceholder;
  };
}

export function commitTickAxisOptions() {
  return {
    autoSkip: false,
    maxRotation: chartCommitTickRotation,
    minRotation: chartCommitTickRotation,
    fontSize: chartCommitTickFontSize(),
    fontFamily: 'Inter',
    fontColor: chartTheme.tickMuted,
    padding: 6,
    callback(value) {
      return value || chartCommitTickPlaceholder;
    },
  };
}
