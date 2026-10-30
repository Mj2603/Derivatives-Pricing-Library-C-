# Derivatives Pricing Library

A C++17 research library for vanilla and selected path-dependent option pricing, calibration experiments, and portfolio risk aggregation.

## Implemented scope

- European Black–Scholes pricing and analytical Greeks.
- Cox–Ross–Rubinstein binomial pricing, including American early exercise.
- Monte Carlo pricing for European and arithmetic Asian options.
- Finite-difference pricing for European options and selected barrier knock-out contracts.
- Yield-curve discounting, strike-volatility interpolation, and implied-volatility root finding.
- SABR calibration and portfolio Greek aggregation.

This is an educational research library, not a validated production pricing or risk system. The supported products and model assumptions are limited. The SABR calibration is experimental; see the source and tests for the current parameterization and coverage.

## Build

Requires CMake 3.16+, a C++17 compiler, and network access on the first configure to fetch GoogleTest.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Examples and benchmark

```bash
./build/price_european 100 100 0.05 0.20 1.0 1
./build/bench_pricing
```

The benchmark executable is a local comparison harness. Runtime figures depend on hardware, compiler, build flags, and system load; do not treat the values in old runs as portable performance claims. For publishable results, record the environment, run repeated trials, and report uncertainty as well as timing.

## Repository layout

- `include/dpl/` — public interfaces
- `src/` — pricing, calibration, market data, and risk implementations
- `tests/` — numerical and behavior checks
- `examples/` — command-line pricing example
- `benchmarks/` — local timing harness
- `docs/architecture.md` — model and convention notes

## Model conventions and limitations

- Greeks use the conventions documented in [architecture notes](docs/architecture.md).
- Numerical engines require independent convergence and reference-price validation for each product and parameter regime.
- The library does not currently provide a complete arbitrage-controlled volatility surface, local-volatility or Heston model, knock-in barrier support, or American Monte Carlo/finite-difference exercise.
- No independent production validation, market-data controls, deployment packaging, or service-level monitoring is included.

## License

MIT. See [LICENSE](LICENSE).
