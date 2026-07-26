import {
  deltaFlatThresholdPct,
  deltaNoiseBandPct,
  deltaAlertThresholdPct,
} from './config.js';

export function classifyDeltaMagnitude(pct) {
  const abs = Math.abs(pct);
  if (abs < deltaFlatThresholdPct) return 'flat';
  if (abs < deltaNoiseBandPct) return 'noise';
  if (abs < deltaAlertThresholdPct) return 'notable';
  return 'alert';
}

export function enrichDelta(delta) {
  if (!delta) return null;
  return {
    ...delta,
    magnitude: classifyDeltaMagnitude(delta.pct),
  };
}

export function computeDeltaAtIndex(dataset, index) {
  if (!dataset || index < 1 || index >= dataset.length) return null;
  const latest = dataset[index].bench.value;
  const previous = dataset[index - 1].bench.value;
  if (!Number.isFinite(previous) || !Number.isFinite(latest) || previous === 0) return null;
  const pct = ((latest - previous) / previous) * 100;
  return {
    pct,
    latest,
    tone: Math.abs(pct) < deltaFlatThresholdPct ? 'flat' : pct > 0 ? 'slower' : 'faster',
  };
}

export function computeVsPreviousDelta(dataset) {
  if (!dataset || dataset.length < 2) return null;
  return computeDeltaAtIndex(dataset, dataset.length - 1);
}

export function formatDeltaShort(delta) {
  if (!delta || delta.tone === 'flat') return '~0%';
  const arrow = delta.tone === 'slower' ? '↑' : '↓';
  return arrow + Math.abs(delta.pct).toFixed(1) + '%';
}

export function applyDeltaChangeSpan(change, delta, baseClass = 'metric-delta', { neutral = false } = {}) {
  if (!delta) {
    change.className = baseClass + ' flat';
    change.textContent = '—';
    return;
  }
  if (delta.tone === 'flat') {
    change.className = baseClass + ' flat';
    change.textContent = '~0%';
    return;
  }

  const magnitude = delta.magnitude ?? classifyDeltaMagnitude(delta.pct);
  const worse = delta.tone === 'slower';
  const classes = [baseClass];

  if (neutral) {
    classes.push('neutral');
  } else if (magnitude === 'noise') {
    classes.push('noise');
  } else if (magnitude === 'notable') {
    classes.push('notable', worse ? 'up' : 'down');
  } else {
    classes.push(worse ? 'up' : 'down');
  }

  change.className = classes.join(' ');
  change.textContent = (worse ? '↗' : '↘') + ' ' + Math.abs(delta.pct).toFixed(1) + '%';
}

export function createDeltaChangeSpan(delta, baseClass = 'metric-delta', { neutral = false } = {}) {
  const change = document.createElement('span');
  applyDeltaChangeSpan(change, delta, baseClass, { neutral });
  return change;
}

export function deltaTooltipText(delta, kind, latestLabel, runnerNote = '') {
  if (!delta) return 'No previous commit to compare against yet.';

  const pctLabel = delta.tone === 'flat'
    ? 'About the same as the previous commit'
    : Math.abs(delta.pct).toFixed(1) + '% '
    + (delta.tone === 'slower' ? 'higher' : 'lower')
    + ' than the previous commit';

  let magnitudeLine = '';
  if (delta.magnitude === 'noise') {
    magnitudeLine = '\n\nWithin typical CI noise band';
  } else if (delta.magnitude === 'notable') {
    magnitudeLine = '\n\nModerate change';
  } else if (delta.magnitude === 'alert') {
    magnitudeLine = '\n\nLarge change — worth investigating';
  }

  const header = latestLabel ? latestLabel + '\n' : '';
  const runnerLine = runnerNote ? '\n\n' + runnerNote.trim() : '';

  if (kind === 'sentinel') {
    return header
      + pctLabel
      + magnitudeLine
      + '\n\nSynthetic host check — not Scyclone code'
      + '\nUse to interpret plugin metric shifts.'
      + runnerLine;
  }

  return header + pctLabel + magnitudeLine + runnerLine;
}
