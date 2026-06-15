import { registerChartPlugins } from './charts.js';
import { chartSync } from './chart-sync.js';
import { chartCommitTooltip, styledTooltip } from './tooltips.js';
import {
  initPageData,
  renderBenchSet,
} from './render.js';

registerChartPlugins();

const main = document.getElementById('main');

main.addEventListener('mouseleave', () => {
  chartSync.clearAll();
  styledTooltip.hide();
  chartCommitTooltip.hide();
});

let chartResizeTimer;
window.addEventListener('resize', () => {
  clearTimeout(chartResizeTimer);
  chartResizeTimer = setTimeout(() => {
    chartSync.resizeCharts();
  }, 150);
});

for (const { dataSet } of initPageData()) {
  renderBenchSet(dataSet, main);
}
