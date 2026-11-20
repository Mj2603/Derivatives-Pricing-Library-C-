# Derivatives Pricing Library

A C++17 research library for vanilla and selected path-dependent option pricing, SABR smile calibration, and portfolio risk aggregation.

## Implemented scope

- European Black–Scholes pricing and analytical Greeks (no dividends / continuous yield).
- Cox–Ross–Rubinstein binomial pricing, including American early exercise.
- Monte Carlo pricing (GBM, antithetic variates, discounted-terminal-spot control variate) for European and arithmetic Asian options.
- Finite-difference pricing in log spot (explicit, implicit, Crank–Nicolson) for European options and up-and-out / down-and-out barrier knock-outs.
- Yield-curve discounting, cubic-spline strike-volatility interpolation, and a bracketing bisection implied-volatility solver.
- SABR: Hagan et al. (2002) lognormal implied-volatility approximation, and a multi-quote least-squares calibration of (alpha, rho, nu) with beta fixed (in-repo Nelder–Mead, bounds enforced by reparameterisation, ATM-anchored initial guess).
- Portfolio Greek aggregation.

This is an educational research library, not a validated production pricing or risk system.

## Build and test

Requires CMake 3.16+, a C++17 compiler, and network access on the first configure to fetch GoogleTest.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The library, tests, examples and benchmark compile with `-Wall -Wextra` without warnings (checked with GCC 13.3). `ctest` runs 8 test executables (33 GoogleTest cases).

## Validation of the SABR implementation

`tests/test_sabr.cpp` checks `sabr_implied_vol` against hard-coded values produced by an independent pure-Python reference, `scripts/sabr_reference.py` (which also cross-checks the closed-form x(z) against numerical integration of dx/dz). Agreement is required to 1e-11 across 11 strike/parameter cases covering beta = 0, 0.5 and 1 with equity-style and rates-style forwards. Further tests cover the ATM closed form, continuity through F = K, input validation, bounds, and calibration:

| Calibration test (beta fixed at the true value) | Result |
|---|---|
| 11-strike noise-free smile, true (alpha, rho, nu) = (2.0, -0.3, 0.4), F=100, T=1 | parameters recovered within the 1e-3 test tolerance (observed error ~1e-6); RMSE < 1e-6 vol |
| 7-strike rates smile, (0.035, -0.25, 0.6), F=3%, T=5 | parameters within 1e-3 (alpha within 1e-4); RMSE < 1e-6 vol |
| 17-strike smile plus 10 bp Gaussian vol noise (seed 7) | RMSE 6.6e-4, below the injected noise RMS 8.4e-4; fitted alpha 1.997, rho -0.286, nu 0.404 |

```bash
python3 scripts/sabr_reference.py                  # reference table used by the tests
./build/sabr_smile                                 # print a smile, then calibrate back from it
./build/sabr_smile 0.03 5.0 0.035 0.5 -0.25 0.6    # forward maturity alpha beta rho nu
```

## Examples

```bash
./build/price_european 100 100 0.05 0.20 1.0 1     # spot strike rate vol maturity is_call
python3 scripts/bs_reference.py 100 100 0.05 0.20 1.0 call
```

Both give 10.450584 (to 6 d.p.) for the at-the-money call.

## Benchmark

```bash
./build/bench_pricing
```

Measured on 2026-11-20 in a shared cloud container: Intel Xeon @ 2.10 GHz, 4 vCPUs, Linux 6.18, GCC 13.3.0, `-O2` Release build, single-threaded code. Seven consecutive runs; the host load average was about 4 during the runs, so expect noise. Each run times a single call per task with no warm-up; this is a rough harness, not a statistically controlled benchmark. **Timings depend on hardware, compiler and load; re-run on your own machine.** The accuracy column is deterministic.

European call, S=K=100, r=5%, sigma=20%, T=1 (Black–Scholes reference 10.4506):

| Task | Accuracy | Time: median (min–max) of 7 runs |
|---|---|---|
| CRR binomial, 500 steps | abs. error 4.0e-3 | 134 µs (134–150) |
| Monte Carlo, 100,000 paths x 100 steps, seed 42 | abs. error 1.2e-2 | 218 ms (205–317) |
| Crank–Nicolson FD, 200 x 200 grid | abs. error 4.8e-3 | 539 µs (475–680) |
| Portfolio Greeks, 5,000 European calls (analytical) | — | 484 µs (465–814) |
| SABR calibration, 17 quotes, beta fixed | RMSE 3.8e-8 vol; parameter error ≤ 1.1e-6 | 386 µs (344–633) |

## Repository layout

- `include/dpl/` — public interfaces
- `src/` — pricing, calibration, market data, and risk implementations
- `tests/` — GoogleTest numerical and behaviour checks
- `examples/` — `price_european` and `sabr_smile` command-line examples
- `benchmarks/` — local timing harness
- `scripts/` — independent Python reference implementations (Black–Scholes, SABR)
- `docs/architecture.md` — model and convention notes

## Model conventions and limitations

- Greeks use the conventions documented in [architecture notes](docs/architecture.md).
- No dividends or continuous dividend yield in any engine.
- No Heston, local-volatility, or other stochastic-volatility pricing engine. SABR is used only as a smile parameterisation (implied vols); options are not priced under SABR dynamics.
- The Hagan formula is an asymptotic approximation: it loses accuracy for long maturities and large vol-of-vol, and can imply negative densities at low strikes. No arbitrage checks are applied to calibrated smiles. The calibrator is a local optimiser started from a seed grid; it does not guarantee a global minimum, and beta is not fitted.
- The volatility "surface" is a single strike smile, not a full arbitrage-controlled surface.
- No knock-in barriers, and no American exercise in Monte Carlo or finite difference.
- Numerical engines require independent convergence and reference-price validation for each product and parameter regime.
- No independent production validation, market-data controls, deployment packaging, or service-level monitoring is included.

## License

MIT. See [LICENSE](LICENSE).
