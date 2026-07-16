import {
  chartCommitTickPadding,
  chartTheme,
  stackedYAxisWidthPx,
  stackedXAxisHeightPx,
} from './config.js';
import {
  benchValueToChart,
  chartYAxisLabel,
} from './format.js';
import {
  chartAspectRatio,
  chartFillColor,
  alignInlineKpiToPlot,
  commitTickAxisOptions,
  configureCommitAxisTicks,
  stackedYAxisMax,
  yAxisTickOptions,
} from './chart-layout.js';
import { chartSync } from './chart-sync.js';
import { chartCommitTooltip } from './tooltips.js';

export function registerChartPlugins() {
  Chart.plugins.register({
    id: 'commitTickLayoutReserve',
    afterLayout(chart) {
      const scale = chart.scales['x-axis-0'];
      if (!scale) return;
      chart._commitTickPadFloor = Math.max(chart._commitTickPadFloor || 0, scale.paddingBottom);
      chart._chartAreaBottomFloor = Math.max(
        chart._chartAreaBottomFloor || 0,
        chart.chartArea.bottom,
      );
    },
    beforeDatasetsDraw(chart) {
      if (chart._chartAreaBottomFloor == null) return;
      if (chart.chartArea.bottom < chart._chartAreaBottomFloor) {
        const delta = chart._chartAreaBottomFloor - chart.chartArea.bottom;
        chart.chartArea.bottom += delta;
        chart.chartArea.height += delta;
      }
    },
  });

  Chart.plugins.register({
    id: 'benchmarkHoverPaint',
    beforeDatasetsDraw(chart) {
      chartSync.paintPointsForChart(chart);
    },
    afterDatasetsDraw(chart) {
      chartSync.paintCommitCrosshair(chart);
    },
  });
}

export function initChart(canvas, meta, dataset, color, block, benchName, {
  layout = 'default',
  showXAxisLabels = true,
  emphasizeXAxisLabels = showXAxisLabels,
  reserveXAxisSpace = false,
} = {}) {
  const yUnit = chartYAxisLabel(meta, dataset.length > 0 ? dataset[0].bench.unit : '');
  const chartData = {
    labels: dataset.map(d => d.commit.id.slice(0, 7)),
    datasets: [{
      label: meta.label,
      data: dataset.map(d => benchValueToChart(meta, d.bench.value)),
      borderColor: color,
      backgroundColor: chartFillColor(color),
      borderWidth: 2.5,
      pointBackgroundColor: color,
      pointBorderColor: '#111111',
      pointBorderWidth: 1,
      pointRadius: 0,
      pointHoverRadius: 7,
      fill: false,
      lineTension: 0,
    }],
  };

  const isStacked = layout === 'stacked';
  const xTicksDisplayed = isStacked ? true : showXAxisLabels;
  const stackedYMax = isStacked
    ? stackedYAxisMax(chartData.datasets[0].data, { kind: meta.kind })
    : undefined;

  const chart = new Chart(canvas, {
    type: 'line',
    data: chartData,
    options: {
      legend: { display: false },
      maintainAspectRatio: !isStacked,
      aspectRatio: isStacked ? 2 : chartAspectRatio(),
      layout: {
        padding: {
          bottom: isStacked ? 0 : (showXAxisLabels ? chartCommitTickPadding : 8),
        },
      },
      elements: {
        point: { hitRadius: 12 },
        line: { borderJoinStyle: 'round' },
      },
      hover: {
        mode: 'index',
        intersect: false,
        animationDuration: 0,
      },
      scales: {
        xAxes: [{
          afterFit(scale) {
            if (isStacked) {
              scale.height = stackedXAxisHeightPx;
            }
          },
          gridLines: {
            display: false,
            drawBorder: false,
          },
          ticks: {
            ...commitTickAxisOptions(),
            display: xTicksDisplayed,
            fontColor: reserveXAxisSpace && !emphasizeXAxisLabels
              ? 'rgba(0,0,0,0)'
              : chartTheme.tickMuted,
          },
        }],
        yAxes: [{
          afterFit(scale) {
            if (isStacked) {
              scale.width = stackedYAxisWidthPx;
            }
          },
          gridLines: {
            color: chartTheme.grid,
            zeroLineColor: chartTheme.grid,
            drawBorder: false,
          },
          ticks: yAxisTickOptions(meta, { stacked: isStacked, yMax: stackedYMax }),
          scaleLabel: {
            display: !isStacked,
            labelString: yUnit,
            fontColor: chartTheme.label,
            fontFamily: 'Inter',
            fontSize: 11,
          },
        }],
      },
      tooltips: {
        enabled: false,
        mode: 'index',
        intersect: false,
        custom(tooltipModel) {
          chartCommitTooltip.update(tooltipModel, this, { dataset, meta, color });
        },
      },
      onClick(_evt, active) {
        if (active.length === 0) return;
        const url = dataset[active[0]._index].commit.url;
        if (url) window.open(url, '_blank');
      },
      onHover(_evt, active) {
        if (active.length === 0) {
          chartSync.clearAll();
          return;
        }
        const index = active[0]._index;
        chartSync.setMetricHover(benchName);
        chartSync.setCommitHover(dataset[index].commit.id);
        chartCommitTooltip.showAt({
          chart: this,
          dataset,
          meta,
          color,
          index,
        });
      },
    },
  });

  configureCommitAxisTicks(
    chart,
    chart.data.labels,
    emphasizeXAxisLabels,
    { hideLabels: reserveXAxisSpace && !emphasizeXAxisLabels },
  );
  chart.update(0);
  if (layout === 'stacked') {
    alignInlineKpiToPlot(chart, block);
  }
  chartSync.registerChart({
    chart,
    dataset,
    baseColor: color,
    block,
    benchName,
    layout,
    showXAxisLabels: xTicksDisplayed,
    emphasizeXAxisLabels,
    reserveXAxisSpace,
  });
  return chart;
}
