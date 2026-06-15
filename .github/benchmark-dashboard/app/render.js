import { chartPalette, buildBenchOrder } from './config.js';
import { collectBenchesPerTestCase, partitionBenches, sortBenches, sortOverviewBenches } from './bench-data.js';
import { benchKey, getBenchMeta, benchInfoTooltipText, formatCompactBenchParts, formatCompactBenchValue, shortSentinelLabel } from './format.js';
import {
  appVsRunnerNote,
  analyzeOverviewMetrics,
  analyzeOverviewMetricsAtIndex,
  maxRunnerDriftPct,
  maxRunnerDriftPctAtIndex,
  resolveVerdictCopy,
  runnerDriftTooltipText,
  verdictLabelTooltip,
} from './analysis.js';
import {
  applyDeltaChangeSpan,
  computeDeltaAtIndex,
  computeVsPreviousDelta,
  createDeltaChangeSpan,
  enrichDelta,
} from './deltas.js';
import { chartSync } from './chart-sync.js';
import { initChart } from './charts.js';
import { styledTooltip } from './tooltips.js';

const chartAnchors = new Map();

function benchChartId(benchName) {
  return 'bench-chart-' + benchKey(benchName);
}

function scrollToChart(benchName) {
  let block = chartAnchors.get(benchName);
  if (!block) {
    block = document.getElementById(benchChartId(benchName));
  }
  if (!block) return;

  const details = block.closest('details.baseline-section');
  const focusChart = () => {
    chartSync.setMetricHover(benchName);
    block.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
  };

  if (details && !details.open) {
    details.open = true;
    requestAnimationFrame(() => requestAnimationFrame(focusChart));
  } else {
    focusChart();
  }
}

function bindKpiNavigation(tile, benchName, tooltipText) {
  tile.setAttribute('role', 'button');
  tile.setAttribute('tabindex', '0');
  if (tooltipText) {
    styledTooltip.bind(tile, () => tooltipText);
  }

  tile.addEventListener('click', () => scrollToChart(benchName));
  tile.addEventListener('keydown', (event) => {
    if (event.key === 'Enter' || event.key === ' ') {
      event.preventDefault();
      scrollToChart(benchName);
    }
  });
}

function appendBaselineDriftMetrics(parent, sentinelBenches) {
  if (!sentinelBenches.length) return null;

  const parts = [];
  const metrics = document.createElement('div');
  metrics.className = 'baseline-drift-metrics';

  sentinelBenches.forEach(([benchName, data]) => {
    const meta = getBenchMeta(benchName);
    const delta = enrichDelta(computeVsPreviousDelta(data));
    const row = document.createElement('div');
    row.className = 'baseline-drift-metric';
    row.appendChild(document.createTextNode(shortSentinelLabel(meta) + ' '));
    const changeEl = createDeltaChangeSpan(delta, 'kpi-delta', { neutral: true });
    row.appendChild(changeEl);
    metrics.appendChild(row);
    parts.push({ data, changeEl });
  });

  parent.appendChild(metrics);
  return { parts };
}

function updateKpiTileAtIndex(entry, commitIndex) {
  const {
    dataset,
    meta,
    numEl,
    unitEl,
    changeEl,
    deltaClass,
    tile,
  } = entry;
  if (!dataset || commitIndex < 0 || commitIndex >= dataset.length) return;

  const delta = enrichDelta(computeDeltaAtIndex(dataset, commitIndex));
  const value = dataset[commitIndex].bench.value;
  const valueParts = formatCompactBenchParts(delta?.latest ?? value, meta);

  numEl.textContent = valueParts.num;
  if (unitEl) {
    unitEl.textContent = valueParts.unit;
    unitEl.hidden = !valueParts.unit;
  }

  applyDeltaChangeSpan(changeEl, delta, deltaClass, { neutral: meta.kind === 'sentinel' });
  tile.classList.toggle(
    'metrics-alert',
    Boolean(delta?.magnitude === 'alert' && meta.kind === 'app'),
  );
}

function createCompactKpiTile(benchName, dataset, color, delta, latest, meta, deltaClass, runnerDriftMax, rankPrimary) {
  const valueParts = formatCompactBenchParts(delta?.latest ?? latest, meta);
  const tile = document.createElement('div');
  tile.className = 'kpi-col';
  tile.style.setProperty('--kpi-color', color);

  if (meta.kind === 'build') tile.classList.add('kpi-col-build');
  if (rankPrimary) tile.classList.add('kpi-col-rank-primary');

  const foot = document.createElement('div');
  foot.className = 'kpi-col-foot';
  const label = document.createElement('span');
  label.className = 'kpi-label';
  label.textContent = meta.kpiLabel || meta.label;
  foot.appendChild(label);
  tile.appendChild(foot);

  const valueRow = document.createElement('div');
  valueRow.className = 'kpi-col-value';

  const num = document.createElement('span');
  num.className = 'kpi-col-num';
  num.textContent = valueParts.num;
  valueRow.appendChild(num);

  let unitEl = null;
  if (valueParts.unit) {
    const unit = document.createElement('span');
    unit.className = 'kpi-col-unit';
    unitEl = unit;
    unit.textContent = valueParts.unit;
    valueRow.appendChild(unit);
  }
  tile.appendChild(valueRow);

  const changeEl = createDeltaChangeSpan(delta, deltaClass, { neutral: meta.kind === 'sentinel' });
  tile.appendChild(changeEl);

  return { tile, numEl: num, unitEl, changeEl };
}

function createStandardKpiTile(benchName, dataset, color, delta, latest, meta, deltaClass) {
  const tile = document.createElement('div');
  tile.className = 'kpi-tile';
  tile.style.setProperty('--kpi-color', color);

  const head = document.createElement('div');
  head.className = 'kpi-head';
  const label = document.createElement('span');
  label.className = 'kpi-label';
  label.textContent = meta.label;
  head.appendChild(label);
  tile.appendChild(head);

  const body = document.createElement('div');
  body.className = 'kpi-body';
  const change = createDeltaChangeSpan(delta, deltaClass, { neutral: meta.kind === 'sentinel' });
  body.appendChild(change);

  const value = document.createElement('span');
  value.className = 'kpi-value';
  value.textContent = formatCompactBenchValue(delta?.latest ?? latest, meta);
  body.appendChild(value);
  tile.appendChild(body);

  return { tile, changeEl: change };
}

function createKpiTile(
  benchName,
  dataset,
  colorIndex,
  {
    runnerDriftMax = 0,
    compact = false,
    rankPrimary = false,
    inline = false,
  } = {},
) {
  const meta = getBenchMeta(benchName);
  const delta = enrichDelta(computeVsPreviousDelta(dataset));
  const latest = dataset.length > 0 ? dataset[dataset.length - 1].bench.value : null;
  const color = chartPalette[colorIndex % chartPalette.length];
  const deltaClass = compact ? 'kpi-col-delta' : 'kpi-delta';
  const tooltipText = benchInfoTooltipText(meta) + appVsRunnerNote(delta, runnerDriftMax);

  let tile;
  let numEl = null;
  let unitEl = null;
  let changeEl = null;

  if (compact) {
    ({ tile, numEl, unitEl, changeEl } = createCompactKpiTile(
      benchName, dataset, color, delta, latest, meta, deltaClass, runnerDriftMax, rankPrimary,
    ));
    if (inline) {
      tile.classList.add('kpi-col--inline');
    }
  } else {
    ({ tile, changeEl } = createStandardKpiTile(
      benchName, dataset, color, delta, latest, meta, deltaClass,
    ));
  }

  bindKpiNavigation(tile, benchName, tooltipText);

  if (delta && delta.magnitude === 'alert' && meta.kind === 'app') {
    tile.classList.add('metrics-alert');
  }

  if (compact) {
    chartSync.registerKpi({
      tile,
      benchName,
      overview: true,
      dataset,
      meta,
      numEl,
      unitEl,
      changeEl,
      deltaClass,
    });
  } else {
    chartSync.registerKpi({ tile, benchName });
  }

  return { tile, color };
}

function createKpiPanel(
  benches,
  startColorIndex,
  {
    runnerDriftMax = 0,
    panelLabel = null,
    overview = false,
    overviewAnalysis = null,
  } = {},
) {
  const panel = document.createElement('div');
  panel.className = 'kpi-panel';

  if (panelLabel) {
    const label = document.createElement('p');
    label.className = 'kpi-panel-label';
    label.textContent = panelLabel;
    panel.appendChild(label);
  }

  const orderedBenches = overview ? sortOverviewBenches(benches) : benches;

  const grid = document.createElement('div');
  grid.className = overview ? 'kpi-strip' : 'kpi-grid';
  if (overview) {
    const hasBuild = orderedBenches.some(([benchName]) => getBenchMeta(benchName).kind === 'build');
    if (hasBuild) {
      grid.classList.add('kpi-strip--grouped');
    }
  }
  grid.setAttribute(
    'aria-label',
    overview ? 'Benchmark change vs previous develop push' : 'Latest benchmark summary',
  );

  const stripWrap = overview ? document.createElement('div') : null;
  if (stripWrap) {
    stripWrap.className = 'kpi-strip-card';
    stripWrap.appendChild(grid);
    panel.appendChild(stripWrap);
  } else {
    panel.appendChild(grid);
  }

  const colors = new Map();
  let colorIndex = startColorIndex;
  for (const [benchName, data] of orderedBenches) {
    const rankPrimary = overviewAnalysis?.primaryBench === benchName
      && getBenchMeta(benchName).kind !== 'build';
    const { tile, color } = createKpiTile(benchName, data, colorIndex++, {
      runnerDriftMax,
      compact: overview,
      rankPrimary,
    });
    colors.set(benchName, color);
    grid.appendChild(tile);
  }

  return { panel, colors, nextColorIndex: colorIndex };
}

function createRunnerContextColumn(sentinelBenches, analysis, overviewBenches) {
  const copy = analysis ? resolveVerdictCopy(analysis) : { status: '', tone: 'neutral' };
  const hasSentinel = sentinelBenches.length > 0;
  const hasVerdict = Boolean(copy.status);
  if (!hasSentinel && !hasVerdict) return null;

  const col = document.createElement('div');
  col.className = 'baseline-context-col';

  let verdictEl = null;
  let verdictWrap = null;
  let tooltipAnalysis = analysis;
  if (hasVerdict) {
    verdictWrap = document.createElement('div');
    verdictWrap.className = 'baseline-interpretation';
    verdictWrap.setAttribute('role', 'status');
    verdictEl = document.createElement('span');
    verdictEl.className = 'overview-lead-verdict overview-lead-verdict--' + copy.tone;
    verdictEl.textContent = copy.status;
    verdictEl.setAttribute('tabindex', '0');
    styledTooltip.bind(verdictEl, () => verdictLabelTooltip(tooltipAnalysis));
    verdictWrap.appendChild(verdictEl);
    col.appendChild(verdictWrap);
  }

  let sentinelParts = [];
  if (hasSentinel) {
    const drift = document.createElement('div');
    drift.className = 'baseline-runner-drift';

    const runnerLabel = document.createElement('span');
    runnerLabel.className = 'baseline-runner-label';
    runnerLabel.textContent = 'Runner drift';
    runnerLabel.setAttribute('tabindex', '0');
    styledTooltip.bind(runnerLabel, runnerDriftTooltipText);
    drift.appendChild(runnerLabel);

    const sentinelDrift = appendBaselineDriftMetrics(drift, sentinelBenches);
    sentinelParts = sentinelDrift?.parts ?? [];
    col.appendChild(drift);
  }

  function updateAtCommitIndex(commitIndex) {
    if (!overviewBenches?.length) return;

    const runnerDriftMax = maxRunnerDriftPctAtIndex(sentinelBenches, commitIndex);
    const analysisAt = analyzeOverviewMetricsAtIndex(
      overviewBenches,
      runnerDriftMax,
      commitIndex,
    );
    tooltipAnalysis = analysisAt;
    if (hasSentinel) {
      sentinelParts.forEach(({ data, changeEl }) => {
        applyDeltaChangeSpan(
          changeEl,
          enrichDelta(computeDeltaAtIndex(data, commitIndex)),
          'kpi-delta',
          { neutral: true },
        );
      });
    }
    if (verdictEl && verdictWrap) {
      const copyAt = resolveVerdictCopy(analysisAt);
      verdictWrap.hidden = !copyAt.status;
      if (copyAt.status) {
        verdictEl.className = 'overview-lead-verdict overview-lead-verdict--' + copyAt.tone;
        verdictEl.textContent = copyAt.status;
      }
    }
  }

  return { root: col, updateAtCommitIndex };
}

function createBaselineSection(sentinel, contextStrip) {
  const details = document.createElement('details');
  details.className = 'content-section baseline-section';

  const summary = document.createElement('summary');
  const row = document.createElement('div');
  row.className = 'chart-row baseline-summary-row';

  const summaryMain = document.createElement('div');
  summaryMain.className = 'baseline-summary-main';

  const summaryTitle = document.createElement('div');
  summaryTitle.className = 'baseline-summary-text';
  summaryTitle.innerHTML = '<h2>Runner baseline</h2>';
  const runnerEnv = document.createElement('p');
  runnerEnv.className = 'baseline-runner-env';
  runnerEnv.textContent = 'macOS arm64 · 44.1 kHz / 512 samples';
  summaryTitle.appendChild(runnerEnv);
  summaryMain.appendChild(summaryTitle);

  const chevron = document.createElement('span');
  chevron.className = 'baseline-chevron';
  chevron.setAttribute('aria-hidden', 'true');
  chevron.textContent = '▸';
  summaryMain.appendChild(chevron);

  row.appendChild(summaryMain);

  if (contextStrip) {
    row.appendChild(contextStrip.root);
  }

  summary.appendChild(row);
  details.appendChild(summary);

  const body = document.createElement('div');
  body.className = 'baseline-body';
  details.appendChild(body);

  const { panel, colors } = createKpiPanel(sentinel, 0, {
    panelLabel: 'Synthetic host checks — compare drift to Scyclone metrics above',
  });
  body.appendChild(panel);

  const graphsElem = document.createElement('div');
  graphsElem.className = 'benchmark-graphs';
  body.appendChild(graphsElem);

  const pending = sentinel.map(([benchName, data]) => {
    const color = colors.get(benchName);
    const { block, canvas, meta } = createChartBlock(benchName, color);
    graphsElem.appendChild(block);
    return { block, canvas, meta, data };
  });

  let baselineReady = false;
  function drawBaselineCharts() {
    if (baselineReady) return;
    baselineReady = true;
    pending.forEach((item, index) => {
      const [benchName] = sentinel[index];
      const color = colors.get(benchName);
      initChart(item.canvas, item.meta, item.data, color, item.block, benchName);
    });
  }

  details.addEventListener('toggle', () => {
    if (details.open) drawBaselineCharts();
  });

  return details;
}

function createSectionHeader(title) {
  const header = document.createElement('div');
  header.className = 'section-header';
  const heading = document.createElement('h2');
  heading.textContent = title;
  header.appendChild(heading);
  return header;
}

function createChartBlock(name, color, { inlineKpi = false } = {}) {
  const meta = getBenchMeta(name);
  const block = document.createElement('div');
  block.className = 'chart-block' + (inlineKpi ? ' chart-block--inline-kpi' : '');
  block.id = benchChartId(name);
  block.setAttribute('aria-label', meta.label + ' trend');
  block.style.setProperty('--chart-color', color);
  chartAnchors.set(name, block);

  const header = document.createElement('div');
  header.className = 'chart-header';
  if (inlineKpi) {
    header.hidden = true;
  }
  block.appendChild(header);

  const titleRow = document.createElement('div');
  titleRow.className = 'chart-title-row';

  const label = document.createElement('h3');
  label.className = 'chart-label';
  label.textContent = meta.label;
  titleRow.appendChild(label);

  if (meta.tooltip) {
    const tip = document.createElement('button');
    tip.type = 'button';
    tip.className = 'info-tip';
    tip.textContent = '?';
    tip.setAttribute('aria-label', meta.label + ': ' + meta.tooltip);
    styledTooltip.bind(tip, meta.tooltip);
    titleRow.appendChild(tip);
  }

  header.appendChild(titleRow);

  const canvas = document.createElement('canvas');
  canvas.className = 'benchmark-chart';
  block.appendChild(canvas);

  return { block, canvas, meta };
}

function buildColorMap(benches, startColorIndex = 0) {
  const colors = new Map();
  let colorIndex = startColorIndex;
  for (const [benchName] of benches) {
    colors.set(benchName, chartPalette[colorIndex % chartPalette.length]);
    colorIndex++;
  }
  return { colors, nextColorIndex: colorIndex };
}

function createBuildSectionHeader() {
  const header = document.createElement('div');
  header.className = 'section-header';

  const heading = document.createElement('h2');
  heading.textContent = 'Build';
  header.appendChild(heading);

  return header;
}

function mergeOverviewBenches(app, build) {
  return [...sortOverviewBenches(app), ...sortBenches(build, buildBenchOrder)];
}

function renderGraphs(parent, benches, startColorIndex, colorMap = null, {
  stacked = false,
  inlineKpi = false,
  runnerDriftMax = 0,
  overviewAnalysis = null,
} = {}) {
  const entries = [...benches];
  let colorIndex = startColorIndex;
  for (let i = 0; i < entries.length; i++) {
    const [benchName, data] = entries[i];
    const color = colorMap?.get(benchName) ?? chartPalette[colorIndex % chartPalette.length];
    const chartOptions = {
      layout: stacked ? 'stacked' : 'default',
      showXAxisLabels: stacked || i === entries.length - 1,
      emphasizeXAxisLabels: stacked ? i === entries.length - 1 : true,
      reserveXAxisSpace: stacked,
    };

    if (inlineKpi) {
      const row = document.createElement('div');
      row.className = 'chart-row chart-row--card';
      row.style.setProperty('--chart-color', color);
      row.setAttribute('aria-label', getBenchMeta(benchName).label + ' benchmark row');

      const { block, canvas, meta } = createChartBlock(benchName, color, { inlineKpi: true });
      row.appendChild(block);

      const rankPrimary = overviewAnalysis?.primaryBench === benchName
        && getBenchMeta(benchName).kind !== 'build';
      const { tile } = createKpiTile(benchName, data, colorIndex, {
        runnerDriftMax,
        compact: true,
        rankPrimary,
        inline: true,
      });
      row.appendChild(tile);
      parent.appendChild(row);

      initChart(canvas, meta, data, color, block, benchName, chartOptions);
    } else {
      const { block, canvas, meta } = createChartBlock(benchName, color);
      parent.appendChild(block);
      initChart(canvas, meta, data, color, block, benchName, chartOptions);
    }

    colorIndex++;
  }
  return colorIndex;
}

export function renderBenchSet(benchSet, main) {
  const { app, build, sentinel } = partitionBenches(benchSet);
  let colorIndex = 0;
  const runnerDriftMax = sentinel.length > 0 ? maxRunnerDriftPct(sentinel) : 0;
  const overview = mergeOverviewBenches(app, build);
  let sharedColors = new Map();
  let overviewAnalysis = null;
  let contextStrip = null;

  const foldPrimary = document.createElement('div');
  foldPrimary.className = 'fold-primary';
  main.appendChild(foldPrimary);

  if (overview.length > 0) {
    overviewAnalysis = analyzeOverviewMetrics(overview, runnerDriftMax);
    const { colors, nextColorIndex } = buildColorMap(sortOverviewBenches(overview), colorIndex);
    sharedColors = colors;
    colorIndex = nextColorIndex;

    contextStrip = createRunnerContextColumn(sentinel, overviewAnalysis, overview);

    chartSync.registerOverview({
      contextStrip,
      updateKpiAtIndex: updateKpiTileAtIndex,
    });
  }

  if (app.length > 0) {
    const section = document.createElement('section');
    section.className = 'content-section';
    foldPrimary.appendChild(section);
    section.appendChild(createSectionHeader('Plugin performance'));

    const graphsElem = document.createElement('div');
    graphsElem.className = 'benchmark-graphs benchmark-graphs--stacked';
    section.appendChild(graphsElem);
    renderGraphs(graphsElem, app, 0, sharedColors, {
      stacked: true,
      inlineKpi: true,
      runnerDriftMax,
      overviewAnalysis,
    });
  }

  if (build.length > 0) {
    const section = document.createElement('section');
    section.className = 'content-section content-section--build';
    main.appendChild(section);
    section.appendChild(createBuildSectionHeader());

    const graphsElem = document.createElement('div');
    graphsElem.className = 'benchmark-graphs benchmark-graphs--stacked';
    section.appendChild(graphsElem);
    renderGraphs(graphsElem, build, 0, sharedColors, {
      stacked: true,
      inlineKpi: true,
      runnerDriftMax,
      overviewAnalysis,
    });
  }

  if (sentinel.length > 0) {
    const footer = document.querySelector('footer');
    const baselineContext = contextStrip
      ?? createRunnerContextColumn(sentinel, null, []);
    footer.insertBefore(
      createBaselineSection(sentinel, baselineContext),
      footer.firstChild,
    );
  }
}

export function initPageData() {
  const data = window.BENCHMARK_DATA;
  const methodologyTip = document.getElementById('methodology-tip');
  styledTooltip.bind(
    methodologyTip,
    'How these benchmarks are measured\n\n'
    + 'Plugin performance\n'
    + 'macOS arm64 CI · 44.1 kHz · 512 samples\n'
    + 'Prepare, Process, Editor\n'
    + 'Each point = mean of 5 repetitions\n\n'
    + 'Build\n'
    + 'Wall-clock Release build of the Benchmark target\n'
    + 'Measured once per CI run',
  );

  document.getElementById('last-update').textContent =
    new Date(data.lastUpdate).toLocaleString(undefined, {
      day: 'numeric',
      month: 'short',
      year: 'numeric',
    });
  const repoLink = document.getElementById('repository-link');
  repoLink.href = data.repoUrl;
  repoLink.textContent = 'GitHub';

  document.getElementById('dl-button').onclick = () => {
    const a = document.createElement('a');
    a.href = 'data:application/json,' + encodeURIComponent(JSON.stringify(data, null, 2));
    a.download = 'benchmark_data.json';
    a.click();
  };

  return Object.keys(data.entries).map(name => ({
    name,
    dataSet: collectBenchesPerTestCase(data.entries[name]),
  }));
}
