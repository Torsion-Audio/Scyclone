import { chartCommitTooltip } from './tooltips.js';
import {
  chartAspectRatio,
  chartFillColor,
  configureCommitAxisTicks,
} from './chart-layout.js';

export function createChartSync() {
  const chartEntries = [];
  const chartEntryByChart = new WeakMap();
  const kpiEntries = [];
  let hoveredCommitId = null;
  let hoveredBenchName = null;
  let clearTimer = null;
  let overviewState = null;

  function findCommitIndex(dataset, commitId) {
    return dataset.findIndex(row => row.commit.id === commitId);
  }

  function dimColor(baseColor) {
    return baseColor.length === 7 ? baseColor + '35' : baseColor;
  }

  function paintPointsForChart(chart) {
    const entry = chartEntryByChart.get(chart);
    if (!entry) return;

    const { dataset, baseColor } = entry;
    const meta = chart.getDatasetMeta(0);
    const n = dataset.length;
    if (!meta?.data?.length || n === 0) return;

    const lastIndex = n - 1;
    const hasCommitHover = Boolean(hoveredCommitId);
    const matchIndex = hasCommitHover ? findCommitIndex(dataset, hoveredCommitId) : -1;
    const activeTooltip = chart.tooltip?._active;
    const hoveredIndexOnChart = activeTooltip?.length ? activeTooltip[0]._index : -1;

    for (let i = 0; i < n; i++) {
      const el = meta.data[i];
      if (!el?._view) continue;

      const isMatch = i === matchIndex;
      const dim = hasCommitHover && !isMatch;
      const isStart = !hasCommitHover && i === 0;
      const isEnd = !hasCommitHover && i === lastIndex;
      const isNativeHover = i === hoveredIndexOnChart;

      let radius;
      let borderWidth;
      let backgroundColor;
      let borderColor;

      if (isMatch) {
        radius = isNativeHover ? 9 : 7;
        borderWidth = 2;
        backgroundColor = '#ffffff';
        borderColor = baseColor;
      } else if (dim) {
        radius = 0;
        borderWidth = 1;
        backgroundColor = baseColor + '55';
        borderColor = '#111111';
      } else if (isEnd) {
        radius = isNativeHover ? 8 : 6;
        borderWidth = 2;
        backgroundColor = baseColor;
        borderColor = '#ffffff';
      } else if (isStart && n > 1) {
        radius = isNativeHover ? 7 : 4;
        borderWidth = 2;
        backgroundColor = baseColor;
        borderColor = '#111111';
      } else {
        radius = isNativeHover ? 7 : 0;
        borderWidth = 1;
        backgroundColor = baseColor;
        borderColor = '#111111';
      }

      el._view.radius = radius;
      el._view.borderWidth = borderWidth;
      el._view.backgroundColor = backgroundColor;
      el._view.borderColor = borderColor;
    }
  }

  function applyChartEntry(entry) {
    const { chart, baseColor, block, benchName } = entry;
    const ds = chart.data.datasets[0];
    const activeMetric = hoveredBenchName === benchName;
    const dimMetric = hoveredBenchName && !activeMetric;

    ds.borderColor = dimMetric ? dimColor(baseColor) : baseColor;
    ds.backgroundColor = chartFillColor(baseColor, dimMetric ? '0a' : '14');

    if (block) {
      block.classList.toggle('metric-dimmed', dimMetric);
      block.classList.toggle('metric-active', activeMetric);
    }

    const showCommitLabels = activeMetric;
    const prevAxis = entry._axisEmphasis;
    entry._axisEmphasis = showCommitLabels;
    configureCommitAxisTicks(chart, chart.data.labels, showCommitLabels);

    if (prevAxis !== showCommitLabels) {
      chart.update({ duration: 0 });
    } else {
      chart.draw();
    }
  }

  function applyKpiEntry(entry) {
    const { tile, benchName } = entry;
    const activeMetric = hoveredBenchName === benchName;
    const dimMetric = hoveredBenchName && !activeMetric;
    tile.classList.toggle('metric-active', activeMetric);
    tile.classList.toggle('metric-dimmed', dimMetric);
  }

  function commitIndexForId(commitId) {
    if (!commitId) return -1;
    for (const entry of chartEntries) {
      const idx = findCommitIndex(entry.dataset, commitId);
      if (idx >= 0) return idx;
    }
    return -1;
  }

  function updateOverviewDisplay() {
    if (!overviewState) return;

    const hoverIndex = hoveredCommitId ? commitIndexForId(hoveredCommitId) : -1;
    const activeCommitIndex = hoverIndex >= 0
      ? hoverIndex
      : ((chartEntries[0]?.dataset?.length ?? 1) - 1);

    kpiEntries.forEach((entry) => {
      if (entry.overview && overviewState.updateKpiAtIndex) {
        overviewState.updateKpiAtIndex(entry, activeCommitIndex);
      }
    });

    overviewState.contextStrip?.updateAtCommitIndex(activeCommitIndex);
  }

  function refresh() {
    chartEntries.forEach(applyChartEntry);
    kpiEntries.forEach(applyKpiEntry);
    updateOverviewDisplay();
  }

  function cancelClear() {
    if (clearTimer) {
      clearTimeout(clearTimer);
      clearTimer = null;
    }
  }

  function scheduleClear() {
    cancelClear();
    clearTimer = setTimeout(() => {
      hoveredCommitId = null;
      hoveredBenchName = null;
      refresh();
      chartCommitTooltip.hide();
    }, 50);
  }

  function setMetricHover(benchName) {
    cancelClear();
    if (!benchName || hoveredBenchName === benchName) return;
    hoveredBenchName = benchName;
    refresh();
  }

  function setCommitHover(commitId) {
    cancelClear();
    if (!commitId || hoveredCommitId === commitId) return;
    hoveredCommitId = commitId;
    refresh();
  }

  function bindHoverTarget(el, onEnter) {
    const enter = () => {
      cancelClear();
      onEnter();
    };
    el.addEventListener('mouseenter', enter);
    el.addEventListener('mouseleave', scheduleClear);
    el.addEventListener('pointerdown', (event) => {
      if (event.pointerType === 'touch') {
        enter();
      }
    });
  }

  return {
    paintPointsForChart,
    registerChart(entry) {
      chartEntries.push(entry);
      chartEntryByChart.set(entry.chart, entry);
      applyChartEntry(entry);
      bindHoverTarget(entry.block, () => setMetricHover(entry.benchName));
    },
    registerKpi(entry) {
      kpiEntries.push(entry);
      applyKpiEntry(entry);
      bindHoverTarget(entry.tile, () => setMetricHover(entry.benchName));
    },
    registerOverview(state) {
      overviewState = state;
    },
    setCommitHover,
    clearCommitHover() {
      if (!hoveredCommitId) return;
      hoveredCommitId = null;
      refresh();
    },
    setMetricHover,
    clearAll() {
      cancelClear();
      hoveredCommitId = null;
      hoveredBenchName = null;
      refresh();
      chartCommitTooltip.hide();
    },
    resizeCharts() {
      const ratio = chartAspectRatio();
      chartEntries.forEach((entry) => {
        entry.chart.options.aspectRatio = ratio;
        const active = hoveredBenchName === entry.benchName;
        configureCommitAxisTicks(entry.chart, entry.chart.data.labels, active);
        entry.chart.resize();
      });
    },
  };
}

export const chartSync = createChartSync();
