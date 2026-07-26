import { benchMeta } from './config.js';

export function benchKey(raw) {
  const m = raw.match(/^BM_([^/]+)/);
  return m ? m[1] : raw;
}

export function formatBenchName(raw) {
  const m = raw.match(/^BM_([^/]+)/);
  if (!m) return raw;
  return m[1]
    .replace(/_/g, ' ')
    .replace(/\b\w/g, c => c.toUpperCase());
}

export function getBenchMeta(raw) {
  const key = benchKey(raw);
  const meta = benchMeta[key];
  if (meta) return meta;
  return {
    label: formatBenchName(raw),
    summary: '',
    tooltip: '',
    kind: 'app',
  };
}

export function shortSentinelLabel(meta) {
  return meta.label.replace(/^Reference\s+/i, '');
}

export function parseCommitMessage(message) {
  if (!message) return { subject: '', body: '' };
  const lines = message.split('\n');
  const subject = lines[0].trim();
  const body = lines.slice(1).join('\n').trim();
  return { subject, body };
}

export function formatCommitTimestamp(iso) {
  if (!iso) return '';
  const date = new Date(iso);
  if (Number.isNaN(date.getTime())) return iso;
  return date.toLocaleString(undefined, {
    day: 'numeric',
    month: 'short',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
  });
}

export function truncateTooltipText(text, maxLen = 320) {
  if (!text || text.length <= maxLen) return text;
  return text.slice(0, maxLen - 1).trimEnd() + '…';
}

export function escapeHtml(text) {
  return String(text)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

/** @param {number} valueNs @param {{ preset?: string, kind?: string }} [opts] */
export function formatNs(valueNs, opts = {}) {
  const { preset = 'full', kind } = opts;
  if (!Number.isFinite(valueNs)) {
    if (preset === 'compact-parts') return { num: 'n/a', unit: '' };
    if (preset === 'axis-ms' || preset === 'axis-s') return '';
    return null;
  }

  const abs = Math.abs(valueNs);

  if (preset === 'axis-ms') {
    const valueMs = valueNs;
    if (abs >= 100) return valueMs.toFixed(0);
    if (abs >= 10) return valueMs.toFixed(1);
    if (abs >= 1) return valueMs.toFixed(1);
    return valueMs.toFixed(2);
  }

  if (preset === 'axis-s') {
    const valueSec = valueNs;
    if (abs >= 100) return valueSec.toFixed(0);
    if (abs >= 10) return valueSec.toFixed(1);
    return valueSec.toFixed(2);
  }

  if (preset === 'compact-parts') {
    if (kind === 'build') {
      if (abs >= 1e9) {
        const n = valueNs / 1e9;
        return { num: n.toFixed(abs >= 10e9 ? 0 : 1), unit: ' s' };
      }
    }
    if (abs >= 1e9) return { num: (valueNs / 1e9).toFixed(1), unit: ' s' };
    if (abs >= 1e6) {
      const n = valueNs / 1e6;
      return { num: n.toFixed(abs >= 10e6 ? 0 : 1), unit: ' ms' };
    }
    if (abs >= 1e3) return { num: (valueNs / 1e3).toFixed(0), unit: ' µs' };
    return { num: valueNs.toFixed(0), unit: ' ns' };
  }

  if (abs >= 1e9) return (valueNs / 1e9).toFixed(2) + ' s';
  if (abs >= 1e6) return (valueNs / 1e6).toFixed(1) + ' ms';
  if (abs >= 1e3) return (valueNs / 1e3).toFixed(1) + ' µs';
  return valueNs.toFixed(0) + ' ns';
}

export function formatBenchValue(valueNs) {
  return formatNs(valueNs, { preset: 'full' });
}

export function formatCompactBenchParts(valueNs, meta) {
  return formatNs(valueNs, { preset: 'compact-parts', kind: meta.kind });
}

export function formatCompactBenchValue(valueNs, meta) {
  const { num, unit } = formatCompactBenchParts(valueNs, meta);
  return num + unit;
}

/** Google Benchmark reports cpu_time in ns/iter; charts use ms for runtime, seconds for build. */
export function benchValueToChart(meta, valueNs) {
  if (meta.kind === 'build') return valueNs / 1e9;
  return valueNs / 1e6;
}

export function chartYAxisLabel(meta, rawUnit) {
  if (meta.kind === 'build') return 's';
  return rawUnit === 'ns/iter' ? 'ms/iter' : (rawUnit || '');
}

export function formatAxisMs(valueMs) {
  return formatNs(valueMs, { preset: 'axis-ms' });
}

export function formatAxisSeconds(valueSec) {
  return formatNs(valueSec, { preset: 'axis-s' });
}

export function formatCommitBodyHtml(body) {
  const trimmed = truncateTooltipText(body);
  if (!trimmed) return '';

  const lines = trimmed.split('\n').map(line => line.trim()).filter(Boolean);
  const listItems = [];
  const paragraphs = [];

  for (const line of lines) {
    if (/^[-*•]\s+/.test(line)) {
      listItems.push(`<li>${escapeHtml(line.replace(/^[-*•]\s+/, ''))}</li>`);
    } else {
      paragraphs.push(`<p class="chart-commit-tooltip__line">${escapeHtml(line)}</p>`);
    }
  }

  let html = '';
  if (listItems.length) {
    html += `<ul class="chart-commit-tooltip__list">${listItems.join('')}</ul>`;
  }
  html += paragraphs.join('');
  return html ? `<div class="chart-commit-tooltip__body">${html}</div>` : '';
}

export function chartTooltipTitle(row) {
  const { subject } = parseCommitMessage(row.commit.message);
  if (subject) return subject;
  return row.commit.id ? row.commit.id.slice(0, 7) : 'Commit';
}

export function chartTooltipValueLabel(row) {
  const { value, range } = row.bench;
  let label = formatBenchValue(value) || '';
  if (range) label += ' (' + range + ')';
  return label;
}

export function buildChartCommitTooltipHtml(row, meta, color, valueLabel) {
  const subject = escapeHtml(chartTooltipTitle(row));
  const bodyHtml = formatCommitBodyHtml(parseCommitMessage(row.commit.message).body);
  const when = formatCommitTimestamp(row.commit.timestamp);
  const hash = row.commit.id ? row.commit.id.slice(0, 7) : '';
  const metaLine = escapeHtml([when, hash].filter(Boolean).join(' · '));

  return [
    `<p class="chart-commit-tooltip__subject">${subject}</p>`,
    '<div class="chart-commit-tooltip__metric">',
    `<span class="chart-commit-tooltip__swatch" style="background:${color}"></span>`,
    `<span class="chart-commit-tooltip__metric-label">${escapeHtml(meta.label)}</span>`,
    `<span class="chart-commit-tooltip__metric-value">${escapeHtml(valueLabel)}</span>`,
    '</div>',
    bodyHtml,
    metaLine ? `<p class="chart-commit-tooltip__meta">${metaLine}</p>` : '',
  ].join('');
}

export function benchInfoTooltipText(meta) {
  return meta.tooltip || meta.summary || '';
}
