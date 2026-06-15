import {
  chartCommitTickPadding,
  chartTheme,
} from './config.js';
import {
  benchValueToChart,
  chartYAxisLabel,
  formatAxisMs,
  formatAxisSeconds,
} from './format.js';
import {
  chartAspectRatio,
  chartFillColor,
  commitTickAxisOptions,
  configureCommitAxisTicks,
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
  });
}

export function initChart(canvas, meta, dataset, color, block, benchName) {
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
      fill: true,
      lineTension: 0.3,
    }],
  };

  const chart = new Chart(canvas, {
    type: 'line',
    data: chartData,
    options: {
      legend: { display: false },
      maintainAspectRatio: true,
      aspectRatio: chartAspectRatio(),
      layout: {
        padding: {
          bottom: chartCommitTickPadding,
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
          gridLines: {
            color: chartTheme.grid,
            zeroLineColor: chartTheme.grid,
            drawBorder: false,
          },
          ticks: commitTickAxisOptions(),
        }],
        yAxes: [{
          gridLines: {
            color: chartTheme.grid,
            zeroLineColor: chartTheme.grid,
            drawBorder: false,
          },
          ticks: {
            fontColor: chartTheme.tick,
            fontFamily: 'Inter',
            fontSize: 11,
            beginAtZero: true,
            callback: value => meta.kind === 'build'
              ? formatAxisSeconds(value)
              : formatAxisMs(value),
          },
          scaleLabel: {
            display: true,
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
        if (active.length === 0) return;
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

  configureCommitAxisTicks(chart, chart.data.labels, false);
  chart.update(0);
  chartSync.registerChart({ chart, dataset, baseColor: color, block, benchName });
  return chart;
}
