from pathlib import Path
import hashlib
import importlib.util
import json
import random
import sys
import time


sys.dont_write_bytecode = True
root = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(root))
spec = importlib.util.spec_from_file_location('bomb_public', root / 'public_test.py')
tests = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tests)
public = set(json.loads((root / 'tests/public.json').read_text())['seeds'])
mask = (1 << 64) - 1


def challenge(seed, phase):
    x = (seed + 0x9e3779b97f4a7c15 * (phase + 1)) & mask
    x = ((x ^ (x >> 30)) * 0xbf58476d1ce4e5b9) & mask
    x = ((x ^ (x >> 27)) * 0x94d049bb133111eb) & mask
    return x ^ (x >> 31)


rng = random.Random(20261014)
seeds = set(range(1024))
seeds.update(rng.getrandbits(64) for _ in range(512))
seeds.update(1 << k for k in range(64))
seeds.update((1 << k) - 1 for k in range(1, 65))
seeds.update((1 << k) + 1 for k in range(64))
seeds.update([20261014, mask - 1, mask - 2])
seeds -= public
coverage = {'phase3': set(), 'phase4': set(), 'phase5': set(), 'phase6': set()}
started = time.monotonic()
for index, seed in enumerate(sorted(seeds), 1):
    error = tests.check_seed(root, seed)
    if error:
        print(f'FAIL seed {seed}: {error}', flush=True)
        raise SystemExit(1)
    coverage['phase3'].add(challenge(seed, 3) & 7)
    coverage['phase4'].add(challenge(seed, 4) & 127)
    coverage['phase5'].add(challenge(seed, 5) & 31)
    coverage['phase6'].add(challenge(seed, 6) & 63)
    if index % 100 == 0:
        print(f'{index}/{len(seeds)} nonpublic seeds: all six phases and Honor passed; '
              f'{time.monotonic() - started:.1f}s', flush=True)

assert len(seeds) == 1693
assert len(coverage['phase3']) == 8
assert len(coverage['phase4']) == 128
assert len(coverage['phase5']) == 32
assert len(coverage['phase6']) == 64
seed_bytes = ''.join(f'{seed}\n' for seed in sorted(seeds)).encode('ascii')
summary = {
    'nonpublic_seeds': len(seeds),
    'passed': len(seeds),
    'coverage': {name: len(values) for name, values in coverage.items()},
    'seed_20261014': 'All six phases and Honor passed',
    'seed_set_sha256': hashlib.sha256(seed_bytes).hexdigest(),
    'bomb_sha256': hashlib.sha256((root / 'bomb').read_bytes()).hexdigest(),
    'defuser_source_sha256': hashlib.sha256((root / 'defuser.c').read_bytes()).hexdigest(),
    'seconds': round(time.monotonic() - started, 1),
}
print(json.dumps(summary, indent=2), flush=True)
