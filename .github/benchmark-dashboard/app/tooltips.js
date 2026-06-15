import {
  buildChartCommitTooltipHtml,
  chartTooltipValueLabel,
} from './format.js';

/**
 * Position a fixed tooltip relative to an anchor rect or chart caret.
 * @param {HTMLElement} el
 * @param {{ left: number, top: number, width?: number, height?: number, caretX?: number, caretY?: number }} anchor
 * @param {{ margin?: number, belowOffset?: number, aboveOffset?: number, hAlign?: 'center' | 'caret' }} [opts]
 */
export function positionFloatingEl(el, anchor, opts = {}) {
  const margin = opts.margin ?? 10;
  const belowOffset = opts.belowOffset ?? margin;
  const aboveOffset = opts.aboveOffset ?? margin;
  const hAlign = opts.hAlign ?? 'center';

  el.hidden = false;
  const tipRect = el.getBoundingClientRect();

  let left;
  if (hAlign === 'caret' && anchor.caretX != null) {
    left = anchor.left + anchor.caretX - (tipRect.width / 2);
  } else {
    const width = anchor.width ?? 0;
    left = anchor.left + (width / 2) - (tipRect.width / 2);
  }

  let top;
  if (anchor.caretY != null) {
    top = anchor.top + anchor.caretY + belowOffset;
    if (top + tipRect.height > window.innerHeight - margin) {
      top = anchor.top + anchor.caretY - tipRect.height - aboveOffset;
    }
  } else {
    top = anchor.top + (anchor.height ?? 0) + belowOffset;
    if (top + tipRect.height > window.innerHeight - margin) {
      top = anchor.top - tipRect.height - aboveOffset;
    }
  }

  if (top < margin) top = margin;
  left = Math.max(margin, Math.min(left, window.innerWidth - tipRect.width - margin));

  el.style.left = left + 'px';
  el.style.top = top + 'px';
}

export function createStyledTooltip() {
  const el = document.createElement('div');
  el.className = 'app-tooltip';
  el.hidden = true;
  el.setAttribute('role', 'tooltip');
  document.body.appendChild(el);

  const showDelay = window.matchMedia('(prefers-reduced-motion: reduce)').matches ? 0 : 260;
  let anchor = null;
  let showTimer = null;

  function position() {
    if (!anchor || el.hidden) return;
    const rect = anchor.getBoundingClientRect();
    positionFloatingEl(el, {
      left: rect.left,
      top: rect.top,
      width: rect.width,
      height: rect.height,
    }, { margin: 10, belowOffset: 10, aboveOffset: 10 });
  }

  function hide() {
    if (showTimer) {
      clearTimeout(showTimer);
      showTimer = null;
    }
    anchor = null;
    el.hidden = true;
    window.removeEventListener('scroll', position, true);
    window.removeEventListener('resize', position);
  }

  function show(target, text) {
    if (!text) return;
    hide();
    anchor = target;
    el.textContent = text;
    el.hidden = false;
    position();
    window.addEventListener('scroll', position, true);
    window.addEventListener('resize', position);
  }

  function bind(target, getText) {
    const resolve = () => (typeof getText === 'function' ? getText() : getText);

    const scheduleShow = () => {
      if (showTimer) clearTimeout(showTimer);
      showTimer = setTimeout(() => show(target, resolve()), showDelay);
    };

    if (target.hasAttribute('title')) {
      target.dataset.styledTooltip = target.getAttribute('title');
      target.removeAttribute('title');
    }

    target.addEventListener('mouseenter', scheduleShow);
    target.addEventListener('mouseleave', hide);
    target.addEventListener('focus', scheduleShow);
    target.addEventListener('blur', hide);
  }

  return { bind, hide };
}

export function createChartCommitTooltip() {
  const root = document.createElement('div');
  root.className = 'chart-commit-tooltip';
  root.hidden = true;
  root.setAttribute('role', 'tooltip');
  root.innerHTML = '<div class="chart-commit-tooltip__inner"></div>';
  document.body.appendChild(root);
  const inner = root.querySelector('.chart-commit-tooltip__inner');
  let activeChart = null;

  function hide() {
    activeChart = null;
    root.hidden = true;
    root.classList.remove('is-visible');
    window.removeEventListener('scroll', reposition, true);
    window.removeEventListener('resize', reposition);
  }

  function reposition() {
    if (!activeChart || root.hidden) return;
    const model = activeChart._activeCommitTooltip;
    if (!model) return;
    place(activeChart, model);
  }

  function place(chart, tooltipModel) {
    const rect = chart.canvas.getBoundingClientRect();
    positionFloatingEl(root, {
      left: rect.left,
      top: rect.top,
      caretX: tooltipModel.caretX,
      caretY: tooltipModel.caretY,
    }, { margin: 12, belowOffset: 14, aboveOffset: 14, hAlign: 'caret' });
  }

  function resolveTooltipIndex(tooltipModel, chart) {
    const fromPoint = tooltipModel.dataPoints?.[0]?.index;
    if (fromPoint != null) return fromPoint;
    if (Number.isInteger(tooltipModel.index)) return tooltipModel.index;
    const active = chart.tooltip?._active;
    if (active?.length && active[0]._index != null) return active[0]._index;
    return null;
  }

  function showAt({ chart, dataset, meta, color, index }) {
    const row = dataset[index];
    if (!row) {
      hide();
      return;
    }

    const point = chart.getDatasetMeta(0)?.data[index];
    if (!point?._view) {
      hide();
      return;
    }

    inner.innerHTML = buildChartCommitTooltipHtml(
      row,
      meta,
      color,
      chartTooltipValueLabel(row),
    );

    const model = {
      opacity: 1,
      caretX: point._view.x,
      caretY: point._view.y,
      index,
    };

    activeChart = chart;
    chart._activeCommitTooltip = model;
    root.hidden = false;
    place(chart, model);
    root.classList.add('is-visible');
    window.removeEventListener('scroll', reposition, true);
    window.removeEventListener('resize', reposition);
    window.addEventListener('scroll', reposition, true);
    window.addEventListener('resize', reposition);
  }

  function update(tooltipModel, chart, ctx) {
    if (!tooltipModel || tooltipModel.opacity <= 0) return;
    const index = resolveTooltipIndex(tooltipModel, chart);
    if (index == null) return;
    showAt({ chart, index, ...ctx });
  }

  return { update, showAt, hide };
}

export const styledTooltip = createStyledTooltip();
export const chartCommitTooltip = createChartCommitTooltip();
