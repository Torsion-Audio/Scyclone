import {
  appBenchOrder,
  buildBenchOrder,
  sentinelBenchOrder,
} from './config.js';
import { benchKey, getBenchMeta } from './format.js';

export { benchKey, getBenchMeta };

export function collectBenchesPerTestCase(entries) {
  const map = new Map();
  for (const entry of entries) {
    const { commit, date, tool, benches } = entry;
    for (const bench of benches) {
      const result = { commit, date, tool, bench };
      const arr = map.get(bench.name);
      if (arr === undefined) map.set(bench.name, [result]);
      else arr.push(result);
    }
  }
  return map;
}

export function sortBenches(entries, order) {
  return [...entries].sort((a, b) => {
    const ia = order.indexOf(benchKey(a[0]));
    const ib = order.indexOf(benchKey(b[0]));
    const ai = ia === -1 ? order.length : ia;
    const bi = ib === -1 ? order.length : ib;
    return ai - bi;
  });
}

export function partitionBenches(benchSet) {
  const app = [];
  const build = [];
  const sentinel = [];
  for (const entry of benchSet.entries()) {
    const kind = getBenchMeta(entry[0]).kind;
    if (kind === 'sentinel') sentinel.push(entry);
    else if (kind === 'build') build.push(entry);
    else app.push(entry);
  }
  return {
    app: sortBenches(app, appBenchOrder),
    build: sortBenches(build, buildBenchOrder),
    sentinel: sortBenches(sentinel, sentinelBenchOrder),
  };
}

export function sortOverviewBenches(benches) {
  return sortBenches([...benches], [...appBenchOrder, ...buildBenchOrder]);
}
