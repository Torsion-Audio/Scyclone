import {
  chartCommitTickPlaceholder,
  chartCommitTickRotation,
  chartTheme,
} from './config.js';
import { formatAxisMs, formatAxisSeconds } from './format.js';

export function chartFillColor(hex, alpha = '14') {
  return hex.length === 7 ? hex + alpha : hex;
}

export function chartAspectRatio(mode = 'default') {
  const w = window.innerWidth;
  if (mode === 'stacked') {
    if (w <= 480) return 2.4;
    if (w <= 720) return 3;
    return 3.75;
  }
  if (w <= 480) return 1.25;
  if (w <= 720) return 1.55;
  return 1.85;
}

export function chartCommitTickFontSize() {
  if (window.innerWidth <= 480) return 9;
  if (window.innerWidth <= 720) return 10;
  return 11;
}

export function configureCommitAxisTicks(chart, labels, emphasized, { hideLabels = false } = {}) {
  const xAxis = chart.options.scales.xAxes[0];

  xAxis.ticks.autoSkip = false;
  xAxis.ticks.maxRotation = chartCommitTickRotation;
  xAxis.ticks.minRotation = chartCommitTickRotation;
  xAxis.ticks.fontSize = chartCommitTickFontSize();
  xAxis.ticks.fontFamily = 'Inter';
  if (hideLabels) {
    xAxis.ticks.fontColor = 'rgba(0,0,0,0)';
  } else {
    xAxis.ticks.fontColor = emphasized ? chartTheme.tick : chartTheme.tickMuted;
  }
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

export function stackedYAxisMax(values, { kind = 'app' } = {}) {
  const nums = values.filter(v => Number.isFinite(v));
  if (!nums.length) return undefined;

  const max = Math.max(...nums);
  if (max <= 0) return 1;

  const padded = max * 1.12;

  if (kind === 'build') {
    if (padded <= 60) return Math.ceil(padded / 10) * 10;
    if (padded <= 300) return Math.ceil(padded / 50) * 50;
    return Math.ceil(padded / 100) * 100;
  }

  if (padded <= 15) return Math.ceil(padded / 5) * 5;
  if (padded <= 60) return Math.ceil(padded / 10) * 10;
  if (padded <= 120) return Math.ceil(padded / 10) * 10;
  if (padded <= 500) return Math.ceil(padded / 50) * 50;
  return Math.ceil(padded / 100) * 100;
}

export function yAxisTickOptions(meta, { stacked = false, yMax } = {}) {
  const compact = stacked || window.innerWidth <= 720;
  const format = value => (meta.kind === 'build' ? formatAxisSeconds(value) : formatAxisMs(value));

  return {
    fontColor: chartTheme.tick,
    fontFamily: 'Inter',
    fontSize: compact ? 10 : 11,
    beginAtZero: true,
    maxTicksLimit: stacked ? 3 : 6,
    padding: stacked ? 2 : 4,
    ...(stacked && yMax != null ? { min: 0, max: yMax } : {}),
    callback(value, index, values) {
      if (stacked && values?.length) {
        const nums = values.map(v => Number(v));
        const maxTick = Math.max(...nums);
        if (Number(value) === maxTick) return '';
      }
      return format(value);
    },
  };
}

export function alignInlineKpiToPlot(chart, block) {
  const row = block.closest('.chart-row');
  if (!row) return;

  const kpi = row.querySelector('.kpi-col--inline');
  if (!kpi) return;

  const rowTop = row.getBoundingClientRect().top;
  const canvasTop = chart.canvas.getBoundingClientRect().top;
  const { top, bottom } = chart.chartArea;
  const plotCenter = canvasTop + (top + bottom) / 2;
  const plotCenterInRow = plotCenter - rowTop;
  const kpiHeight = kpi.getBoundingClientRect().height;

  kpi.style.marginTop = `${Math.max(0, plotCenterInRow - kpiHeight / 2)}px`;
}
