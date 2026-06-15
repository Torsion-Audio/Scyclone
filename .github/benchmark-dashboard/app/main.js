import { registerChartPlugins } from './charts.js';
import { chartSync } from './chart-sync.js';
import { chartCommitTooltip, styledTooltip } from './tooltips.js';
import {
  initPageData,
  renderBenchSet,
  syncFoldPrimaryMinHeight,
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
    syncFoldPrimaryMinHeight();
  }, 150);
});

for (const { dataSet } of initPageData()) {
  renderBenchSet(dataSet, main);
}

requestAnimationFrame(() => {
  requestAnimationFrame(syncFoldPrimaryMinHeight);
});
