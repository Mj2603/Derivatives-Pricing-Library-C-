# Pricing Library Architecture

The library is a C++17 research codebase for option pricing, SABR smile calibration, and portfolio Greeks. It separates instrument definitions and market inputs from numerical pricing engines.

## Pricing flow

```text
Instrument + market inputs
  ├── spot
  ├── discount curve (optional)
  └── volatility surface (optional)
          |
          v
     Pricing engine
          |
          ├── price
          └── supported Greeks / risk aggregation
```

When supplied, `MarketData` provides rates through `discount_factor(T)` and strike volatility through `volatility(strike)`. Otherwise, engines use the inputs documented by their APIs.

## Engine and product coverage

| Product | Implemented method |
|---|---|
| European options | Black–Scholes analytical, CRR binomial, Monte Carlo, and finite difference |
| American options | CRR binomial with early exercise |
| Selected barrier knock-out options | Finite difference |
| Arithmetic Asian options | Monte Carlo |

The methods are not interchangeable across every product. In particular, American exercise is implemented only in the binomial engine; barrier support is limited to selected knock-outs; and arithmetic Asian pricing is implemented in Monte Carlo.

## Numerical conventions

- Analytical Black–Scholes Greeks are provided for European options.
- Theta is expressed per calendar day.
- Vega and rho are expressed per one-percentage-point volatility and rate bump, respectively.
- The finite-difference engine uses log spot, $x = \ln(S)$, and supports explicit, implicit, and Crank–Nicolson schemes.
- Monte Carlo uses geometric Brownian motion with antithetic variates and a discounted-terminal-spot control variate.

## Calibration and risk

### SABR

`sabr_implied_vol` implements the Hagan, Kumar, Lesniewski & Woodward (2002) lognormal approximation:

- z = (nu / alpha) (FK)^((1-beta)/2) ln(F/K)
- x(z) = ln[(sqrt(1 - 2 rho z + z^2) + z - rho) / (1 - rho)], evaluated with `log1p` and a cancellation-free rearrangement for z < rho
- sigma = alpha / {(FK)^((1-beta)/2) [1 + (1-beta)^2 ln^2(F/K) / 24 + (1-beta)^4 ln^4(F/K) / 1920]} · z / x(z) · [1 + ((1-beta)^2 alpha^2 / (24 (FK)^(1-beta)) + rho beta nu alpha / (4 (FK)^((1-beta)/2)) + (2 - 3 rho^2) nu^2 / 24) T]
- for |z| < 1e-4, z / x(z) uses its third-order Taylor series 1 - rho z / 2 + (2 - 3 rho^2) z^2 / 12 + rho (5 - 6 rho^2) z^3 / 24, so the smile is continuous through F = K and reduces exactly to the ATM formula.

`calibrate_sabr(quotes, beta, options)` fits (alpha, rho, nu) to any number (≥ 3) of quotes by minimising the mean squared implied-vol error. It uses an in-repo Nelder–Mead simplex on unconstrained variables (alpha = exp(u), rho = rho_bound · tanh(v), nu = nu_max · logistic(w)), seeded from a 5 x 4 (rho, nu) grid where alpha is solved so the quote closest to ATM is matched exactly, followed by two restarts from the best point. It returns the parameters, RMSE, iteration count and a convergence flag. Beta is supplied by the user and not fitted.

Validation: `tests/test_sabr.cpp` compares against values from the independent Python reference `scripts/sabr_reference.py` and checks parameter recovery from synthetic smiles, with and without noise. Results are summarised in the README.

### Risk

`RiskEngine` aggregates analytical Greeks across a book of European positions.

## Validation boundaries

The library requires independent convergence and reference-price checks for each product and parameter regime. It does not provide a full arbitrage-controlled volatility surface, Heston or local-volatility models, dividends, knock-in barriers, American Monte Carlo/finite-difference exercise, or production market-data controls. See the [README](../README.md) for build instructions, examples, measured benchmark results (with the machine they were measured on), and the complete limitations.
