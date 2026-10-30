# Pricing Library Architecture

The library is a C++17 research codebase for option pricing, volatility calibration experiments, and portfolio Greeks. It separates instrument definitions and market inputs from numerical pricing engines.

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

The repository includes SABR calibration experiments and portfolio Greek aggregation. These are research implementations; users should inspect the model parameterization and tests before applying them to a new market or instrument set.

## Validation boundaries

The library requires independent convergence and reference-price checks for each product and parameter regime. It does not provide a full arbitrage-controlled volatility surface, Heston or local-volatility models, knock-in barriers, American Monte Carlo/finite-difference exercise, or production market-data controls. See the [README](../README.md) for build instructions, examples, benchmarks, and the complete limitations.
