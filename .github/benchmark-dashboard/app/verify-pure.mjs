import assert from 'node:assert/strict';
import { fileURLToPath, pathToFileURL } from 'node:url';
import path from 'node:path';

const appDir = path.dirname(fileURLToPath(import.meta.url));
const importFromApp = (file) => import(pathToFileURL(path.join(appDir, file)).href);

const { formatNs, formatCompactBenchParts, benchValueToChart } = await importFromApp('format.js');
const { computeDeltaAtIndex, enrichDelta, classifyDeltaMagnitude } = await importFromApp('deltas.js');
const { analyzeOverviewMetricsAtIndex, resolveVerdictCopy } = await importFromApp('analysis.js');

const buildMeta = { kind: 'build', label: 'Compile Time' };
const appMeta = { kind: 'app', label: 'Process' };

assert.equal(formatNs(2.5e6, { preset: 'full' }), '2.5 ms');
assert.deepEqual(formatCompactBenchParts(178e9, buildMeta), { num: '178', unit: ' s' });
assert.equal(benchValueToChart(appMeta, 1.18e7), 11.8);

const dataset = [
  { commit: { id: 'a' }, bench: { value: 100 } },
  { commit: { id: 'b' }, bench: { value: 110 } },
];
const delta = enrichDelta(computeDeltaAtIndex(dataset, 1));
assert.equal(delta.tone, 'slower');
assert.equal(classifyDeltaMagnitude(10), 'alert');

const benches = [['BM_process_block/min_time:2.000_mean', dataset]];
const analysis = analyzeOverviewMetricsAtIndex(benches, 1.5, 1);
assert.equal(resolveVerdictCopy(analysis).status, 'Likely regression');

console.log('verify-pure: ok');
