import { getBenchMeta } from './format.js';
import { sortOverviewBenches } from './bench-data.js';
import {
  classifyDeltaMagnitude,
  computeDeltaAtIndex,
  enrichDelta,
  formatDeltaShort,
} from './deltas.js';

export function maxRunnerDriftPctAtIndex(sentinelBenches, commitIndex) {
  let max = 0;
  for (const [, data] of sentinelBenches) {
    const d = computeDeltaAtIndex(data, commitIndex);
    if (d && d.tone !== 'flat') {
      max = Math.max(max, Math.abs(d.pct));
    }
  }
  return max;
}

export function maxRunnerDriftPct(sentinelBenches) {
  if (!sentinelBenches.length) return 0;
  const [, data] = sentinelBenches[0];
  const commitIndex = data.length > 0 ? data.length - 1 : 0;
  return maxRunnerDriftPctAtIndex(sentinelBenches, commitIndex);
}

export function classifyAppVsRunner(delta, maxRunnerDrift) {
  if (!delta || delta.tone === 'flat' || delta.magnitude === 'noise') return null;
  if (!Number.isFinite(maxRunnerDrift)) return 'uncertain';
  const appAbs = Math.abs(delta.pct);
  if (appAbs > maxRunnerDrift + 3) return 'likely-code';
  if (appAbs <= maxRunnerDrift + 1) return 'likely-noise';
  return 'uncertain';
}

export function appVsRunnerNote(delta, maxRunnerDrift) {
  const kind = classifyAppVsRunner(delta, maxRunnerDrift);
  if (kind === 'likely-code') {
    return '\n\nLikely a Scyclone change — larger than runner drift.';
  }
  if (kind === 'likely-noise') {
    return '\n\nMay be shared CI noise — similar to runner drift.';
  }
  return '';
}

export function runnerHintLabel(kind) {
  if (kind === 'likely-code') return 'Likely Scyclone change';
  if (kind === 'likely-noise') return 'May be CI noise';
  if (kind === 'uncertain') return 'Uncertain vs runner';
  return '';
}

export function deltaSeverityRank(delta) {
  if (!delta || delta.tone === 'flat') return 0;
  const mag = delta.magnitude ?? classifyDeltaMagnitude(delta.pct);
  const magRank = { noise: 1, notable: 2, alert: 3 }[mag] ?? 0;
  if (magRank === 0) return 0;
  return magRank * 1000 + Math.abs(delta.pct);
}

export function analyzeOverviewMetricsAtIndex(benches, runnerDriftMax, commitIndex) {
  const entries = [];
  for (const [benchName, data] of benches) {
    const meta = getBenchMeta(benchName);
    const delta = enrichDelta(computeDeltaAtIndex(data, commitIndex));
    const runnerKind = meta.kind === 'app'
      ? classifyAppVsRunner(delta, runnerDriftMax)
      : null;
    entries.push({
      benchName,
      meta,
      delta,
      runnerKind,
      severity: deltaSeverityRank(delta),
    });
  }

  const appEntries = entries.filter(entry => entry.meta.kind === 'app');
  const actionable = appEntries.filter(
    entry => entry.delta
      && (entry.delta.magnitude === 'notable' || entry.delta.magnitude === 'alert'),
  );
  const worstActionable = actionable.length > 0
    ? [...actionable].sort((a, b) => b.severity - a.severity)[0]
    : null;
  const primaryBench = worstActionable?.benchName ?? null;
  const secondaryBenches = new Set(
    actionable
      .filter(entry => entry.benchName !== primaryBench)
      .map(entry => entry.benchName),
  );

  let verdictKind = 'ok';
  if (!entries.some(entry => entry.delta)) {
    verdictKind = 'no-baseline';
  } else if (actionable.length === 0) {
    verdictKind = 'ok';
  } else {
    const codeCount = actionable.filter(entry => entry.runnerKind === 'likely-code').length;
    const noiseCount = actionable.filter(entry => entry.runnerKind === 'likely-noise').length;
    if (codeCount > 0 && noiseCount === 0) {
      verdictKind = 'investigate-code';
    } else if (noiseCount > 0 && codeCount === 0) {
      verdictKind = 'investigate-noise';
    } else if (codeCount > 0 || noiseCount > 0) {
      verdictKind = 'investigate-mixed';
    } else {
      verdictKind = 'investigate-uncertain';
    }
  }

  return {
    entries,
    primaryBench,
    secondaryBenches,
    actionable,
    worstActionable,
    verdictKind,
    runnerDriftMax,
  };
}

export function analyzeOverviewMetrics(benches, runnerDriftMax) {
  const [, data] = benches[0] ?? [];
  const commitIndex = data?.length ? data.length - 1 : 0;
  return analyzeOverviewMetricsAtIndex(benches, runnerDriftMax, commitIndex);
}

export function resolveVerdictCopy(analysis) {
  const { verdictKind, actionable } = analysis;

  if (verdictKind === 'no-baseline') {
    return { tone: 'neutral', status: 'No baseline yet' };
  }

  if (verdictKind === 'ok') {
    return { tone: 'ok', status: 'Stable' };
  }

  const allFaster = actionable.every(entry => entry.delta.tone === 'faster');
  if (allFaster) {
    return { tone: 'ok', status: 'Faster' };
  }

  const statusByKind = {
    'investigate-code': 'Likely regression',
    'investigate-noise': 'Probably CI noise',
    'investigate-mixed': 'Mixed signal',
    'investigate-uncertain': 'Worth checking',
  };

  const toneByKind = {
    'investigate-code': 'alert',
    'investigate-noise': 'watch',
    'investigate-mixed': 'watch',
    'investigate-uncertain': 'watch',
  };

  return {
    tone: toneByKind[verdictKind] || 'neutral',
    status: statusByKind[verdictKind] || '',
  };
}

export function overviewLeadTooltip(analysis) {
  const { actionable, runnerDriftMax } = analysis;
  if (actionable.length === 0) return '';

  const lines = ['Changes vs the previous develop push.', ''];
  for (const item of actionable) {
    const hint = runnerHintLabel(item.runnerKind);
    const suffix = hint ? ` (${hint})` : '';
    lines.push(`${item.meta.label} ${formatDeltaShort(item.delta)}${suffix}`);
  }
  if (Number.isFinite(runnerDriftMax)) {
    lines.push('', `Runner drift peak: ${runnerDriftMax.toFixed(1)}%`);
  }
  return lines.join('\n');
}

export function runnerDriftTooltipText() {
  return 'Runner drift on this CI machine.\n\n'
    + 'Synthetic CPU and memory checks — not Scyclone code.\n\n'
    + 'If plugin metrics shift by a similar amount,\n'
    + 'the change may be runner noise rather than a code regression.';
}

export function verdictLabelTooltip(analysis) {
  const copy = resolveVerdictCopy(analysis);
  const introByStatus = {
    'No baseline yet': 'First benchmark on this chart — no previous commit to compare against.',
    'Stable': 'Prepare, Process, and Editor are within the usual noise band (< ~3% vs the previous push).',
    'Faster': 'Notable improvements in plugin metrics vs the previous push.',
    'Likely regression': 'Notable moves (≥ ~3%) all look larger than runner drift — likely a Scyclone change worth investigating.',
    'Probably CI noise': 'Notable moves track runner CPU/memory drift — may be shared CI noise, not your code.',
    'Mixed signal': 'Some metrics outpace runner drift (likely code), others track it (likely noise). Treat each metric on its own.',
    'Worth checking': 'Something moved notably, but vs runner drift it sits in the gray zone — compare the charts and recent commits.',
  };

  const lines = [introByStatus[copy.status] || copy.status];
  const breakdown = overviewLeadTooltip(analysis);
  if (breakdown) {
    lines.push('', '—', '', breakdown);
  }
  return lines.join('\n');
}
