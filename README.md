# Derivatives Pricing Library (C++)

Object-oriented derivatives pricing framework. Supports analytical, binomial tree, Monte Carlo, and finite-difference methods for European and American vanillas, barrier options (FD), and arithmetic Asian options (MC). Includes Greeks, 1D strike vol surface interpolation, SABR calibration, and portfolio risk aggregation.

## Requirements

- CMake 3.16+
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Test

```bash
cd build
ctest --output-on-failure
```

## Run

```bash
./price_european 100 100 0.05 0.20 1.0 1
./bench_pricing
```

Arguments for `price_european`: spot strike rate vol maturity is_call(0/1).

## Directory Layout

```
include/dpl/     Public headers
src/             Implementation
tests/           Unit tests
benchmarks/      Method comparison
examples/        CLI pricing tool
docs/            Architecture notes
scripts/         Python BS reference
```

## Results

ATM European call: S=100, K=100, r=5%, σ=20%, T=1yr.

| Method | Price | Error | Runtime |
|--------|-------|-------|---------|
| Analytical | 10.4506 | — | 2 μs |
| Binomial (500) | 10.4466 | 0.0040 | 208 μs |
| Monte Carlo (100k) | 10.4381 | 0.0125 | 398 ms |
| Crank-Nicolson (200×200) | 10.4554 | 0.0048 | 769 μs |

Portfolio risk (5000 options): < 1 ms.

## Limitations

- `VolSurface`: 1D cubic spline over strike; passed into `MarketData` for strike-dependent vol
- `YieldCurve`: flat or term-structure discounting via `MarketData`
- Barrier options: Down-and-out and up-and-out only, priced by finite difference
- Asian options: arithmetic average, Monte Carlo only
- SABR calibration: grid search, single-strike fit

## License

MIT
