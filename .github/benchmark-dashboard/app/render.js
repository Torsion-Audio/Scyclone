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

function appendOverviewLeadSep(parent) {
  parent.appendChild(document.createTextNode(' '));
  const sep = document.createElement('span');
  sep.className = 'overview-lead-sep';
  sep.setAttribute('aria-hidden', 'true');
  sep.textContent = '·';
  parent.appendChild(document.createTextNode(' '));
  parent.appendChild(sep);
  parent.appendChild(document.createTextNode(' '));
}

function appendSentinelDrift(parent, sentinelBenches) {
  if (!sentinelBenches.length) return null;

  const parts = [];
  sentinelBenches.forEach(([benchName, data], index) => {
    if (index > 0) {
      parent.appendChild(document.createTextNode(' · '));
    }
    const meta = getBenchMeta(benchName);
    const delta = enrichDelta(computeVsPreviousDelta(data));
    parent.appendChild(document.createTextNode(shortSentinelLabel(meta) + ' '));
    const changeEl = createDeltaChangeSpan(delta, 'kpi-delta', { neutral: true });
    parent.appendChild(changeEl);
    parts.push({ data, changeEl });
  });

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

function createOverviewContextStrip(sentinelBenches, analysis, overviewBenches) {
  const copy = resolveVerdictCopy(analysis);
  const hasSentinel = sentinelBenches.length > 0;
  const hasVerdict = Boolean(copy.status);
  if (!hasSentinel && !hasVerdict) return null;

  const line = document.createElement('p');
  line.className = 'runner-context';
  line.setAttribute('role', 'status');

  let sentinelParts = [];
  if (hasSentinel) {
    const runnerLabel = document.createElement('span');
    runnerLabel.className = 'runner-context-label';
    runnerLabel.textContent = 'Runner drift';
    runnerLabel.setAttribute('tabindex', '0');
    styledTooltip.bind(runnerLabel, runnerDriftTooltipText);
    line.appendChild(runnerLabel);
    line.appendChild(document.createTextNode(' '));

    const drift = appendSentinelDrift(line, sentinelBenches);
    sentinelParts = drift?.parts ?? [];
  }

  let verdictEl = null;
  let verdictWrap = null;
  let tooltipAnalysis = analysis;
  if (hasVerdict) {
    verdictWrap = document.createElement('span');
    verdictWrap.className = 'overview-verdict-wrap';
    if (hasSentinel) appendOverviewLeadSep(verdictWrap);
    verdictEl = document.createElement('span');
    verdictEl.className = 'overview-lead-verdict overview-lead-verdict--' + copy.tone;
    verdictEl.textContent = copy.status;
    verdictEl.setAttribute('tabindex', '0');
    styledTooltip.bind(verdictEl, () => verdictLabelTooltip(tooltipAnalysis));
    verdictWrap.appendChild(verdictEl);
    line.appendChild(verdictWrap);
  }

  function updateAtCommitIndex(commitIndex) {
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

  return { line, updateAtCommitIndex };
}

function createBaselineSummaryDrift(sentinelBenches) {
  const wrap = document.createElement('span');
  wrap.className = 'baseline-summary-drift';
  appendSentinelDrift(wrap, sentinelBenches);
  return wrap;
}

function createSectionHeader(title) {
  const header = document.createElement('div');
  header.className = 'section-header';
  const heading = document.createElement('h2');
  heading.textContent = title;
  header.appendChild(heading);
  return header;
}

function createChartBlock(name, color) {
  const meta = getBenchMeta(name);
  const block = document.createElement('div');
  block.className = 'chart-block';
  block.id = benchChartId(name);
  block.setAttribute('aria-label', meta.label + ' trend');
  block.style.setProperty('--chart-color', color);
  chartAnchors.set(name, block);

  const header = document.createElement('div');
  header.className = 'chart-header';
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

function createBuildSectionHeader(buildBenches) {
  const header = document.createElement('div');
  header.className = 'section-header';

  const heading = document.createElement('h2');
  heading.textContent = 'Build';
  header.appendChild(heading);

  if (buildBenches.length > 0) {
    const [, data] = buildBenches[0];
    const meta = getBenchMeta(buildBenches[0][0]);
    const delta = enrichDelta(computeVsPreviousDelta(data));
    const latest = data.length > 0 ? data[data.length - 1].bench.value : null;
    const summary = document.createElement('span');
    summary.className = 'build-summary-delta';
    summary.appendChild(document.createTextNode(
      'Compile ' + formatCompactBenchValue(delta?.latest ?? latest, meta) + ' ',
    ));
    summary.appendChild(createDeltaChangeSpan(delta, 'kpi-delta'));
    header.appendChild(summary);
  }

  return header;
}

function mergeOverviewBenches(app, build) {
  return [...sortOverviewBenches(app), ...sortBenches(build, buildBenchOrder)];
}

function renderGraphs(parent, benches, startColorIndex, colorMap = null) {
  let colorIndex = startColorIndex;
  for (const [benchName, data] of benches) {
    const color = colorMap?.get(benchName) ?? chartPalette[colorIndex % chartPalette.length];
    const { block, canvas, meta } = createChartBlock(benchName, color);
    parent.appendChild(block);
    initChart(canvas, meta, data, color, block, benchName);
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

  const foldPrimary = document.createElement('div');
  foldPrimary.className = 'fold-primary';
  main.appendChild(foldPrimary);

  if (overview.length > 0) {
    const overviewBlock = document.createElement('div');
    overviewBlock.className = 'overview-block';
    foldPrimary.appendChild(overviewBlock);

    const overviewLayout = document.createElement('div');
    overviewLayout.className = 'overview-layout';
    overviewBlock.appendChild(overviewLayout);

    const overviewAnalysis = analyzeOverviewMetrics(overview, runnerDriftMax);

    const metricsGroup = document.createElement('div');
    metricsGroup.className = 'overview-metrics';
    overviewLayout.appendChild(metricsGroup);

    const { panel, colors, nextColorIndex } = createKpiPanel(overview, colorIndex, {
      runnerDriftMax,
      overview: true,
      overviewAnalysis,
    });
    metricsGroup.appendChild(panel);

    const contextStrip = createOverviewContextStrip(sentinel, overviewAnalysis, overview);
    if (contextStrip) {
      metricsGroup.appendChild(contextStrip.line);
    }

    chartSync.registerOverview({
      contextStrip,
      updateKpiAtIndex: updateKpiTileAtIndex,
    });

    sharedColors = colors;
    colorIndex = nextColorIndex;
  }

  if (app.length > 0) {
    const section = document.createElement('section');
    section.className = 'content-section';
    foldPrimary.appendChild(section);
    section.appendChild(createSectionHeader('Plugin performance'));

    const graphsElem = document.createElement('div');
    graphsElem.className = 'benchmark-graphs';
    section.appendChild(graphsElem);
    colorIndex = renderGraphs(graphsElem, app, colorIndex - overview.length, sharedColors);
  }

  if (build.length > 0) {
    const section = document.createElement('section');
    section.className = 'content-section content-section--build';
    main.appendChild(section);
    section.appendChild(createBuildSectionHeader(build));

    const graphsElem = document.createElement('div');
    graphsElem.className = 'benchmark-graphs';
    section.appendChild(graphsElem);
    colorIndex = renderGraphs(graphsElem, build, colorIndex - build.length, sharedColors);
  }

  if (sentinel.length > 0) {
    const details = document.createElement('details');
    details.className = 'content-section baseline-section';

    const summary = document.createElement('summary');
    const summaryTitle = document.createElement('div');
    summaryTitle.className = 'baseline-summary-text';
    summaryTitle.innerHTML = '<h2>Runner baseline</h2>';
    const runnerEnv = document.createElement('p');
    runnerEnv.className = 'baseline-runner-env';
    runnerEnv.textContent = 'macOS arm64 · 44.1 kHz / 512 samples';
    summaryTitle.appendChild(runnerEnv);
    summary.appendChild(summaryTitle);

    const summaryActions = document.createElement('div');
    summaryActions.className = 'baseline-summary-actions';
    summaryActions.appendChild(createBaselineSummaryDrift(sentinel));
    const chevron = document.createElement('span');
    chevron.className = 'baseline-chevron';
    chevron.setAttribute('aria-hidden', 'true');
    chevron.textContent = '▸';
    summaryActions.appendChild(chevron);
    summary.appendChild(summaryActions);
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

    main.appendChild(details);
  }
}

export function syncFoldPrimaryMinHeight() {
  const fold = document.querySelector('.fold-primary');
  if (!fold) return;
  const top = fold.getBoundingClientRect().top;
  if (!Number.isFinite(top) || top < 0) return;
  const buffer = 32;
  fold.style.minHeight = `calc(100dvh - ${Math.ceil(top)}px + ${buffer}px)`;
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
