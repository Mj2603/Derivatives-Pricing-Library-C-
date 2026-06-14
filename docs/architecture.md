# Architecture

## Overview

Instruments hold contract terms. Engines implement numerical methods. `MarketData` carries spot, discounting (`YieldCurve` optional), and volatility (flat or `VolSurface` by strike).

## Modules

- `core/` — `Option`, `MarketData`, `Greeks`
- `instruments/` — European, American, barrier, Asian
- `market/` — `YieldCurve`, `VolSurface` (1D strike spline)
- `engines/` — analytical, binomial, Monte Carlo, finite difference
- `greeks/` — Black-Scholes closed form
- `calibration/` — implied vol (Newton), SABR (grid search)
- `risk/` — portfolio Greek aggregation

## Engine coverage

| Instrument | Analytical | Binomial | MC | FD |
|------------|------------|----------|----|----|
| European | yes | yes | yes | yes |
| American | — | yes | — | — |
| Barrier (out) | — | — | — | yes |
| Asian (arithmetic) | — | — | yes | — |

## Market data wiring

`AnalyticalEngine` and `MonteCarloEngine` read:

- `market.discount_factor(T)` from flat rate or `YieldCurve`
- `market.volatility(strike)` from flat vol or `VolSurface`

## Numerics notes

**Binomial:** CRR with `u = exp(σ√Δt)`, backward induction, early exercise for American.

**Monte Carlo:** GBM with Euler steps. Antithetic variates negate shocks. Control variate uses discounted terminal spot (`E[e^{-rT}S_T] = S_0`).

**Finite difference:** Black-Scholes PDE in log-space `x = ln S` for uniform mesh near the strike. Explicit, implicit, and Crank-Nicolson via Thomas algorithm.

**Greeks:** Theta per calendar day; vega and rho per 1% move.

## Benchmarks

See `benchmarks/bench_pricing.cpp`. ATM call reference: 10.4506 (S=K=100, r=5%, σ=20%, T=1yr).
