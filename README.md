# Derivatives Pricing Library

C++17 library for pricing European and American vanillas, knock-out barriers, and arithmetic Asian options. Black-Scholes Greeks, 1D vol spline, implied vol root-finding, SABR fit, portfolio greek aggregation.

## Requirements

CMake 3.16+, C++17.

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
ctest --output-on-failure
```

## Example

```bash
./price_european 100 100 0.05 0.20 1.0 1    
./bench_pricing
```

## Layout

- `include/dpl`, `src` — library
- `tests` — gtest
- `benchmarks`, `examples`, `scripts`

## Benchmarks

ATM call, S=K=100, r=5%, sigma=20%, T=1yr. Reference: 10.4506.

| Method | Error | Time |
|--------|-------|------|
| Binomial 500 steps | 0.004 | 208 us |
| MC 100k paths | 0.013 | 398 ms |
| Crank-Nicolson 200x200 | 0.005 | 769 us |

5000-option portfolio greeks: under 1 ms.

## Scope

Implemented: European/American (tree), barrier knock-out (FD), Asian arithmetic (MC), flat or curve discounting, strike vol from spline.

Not done: 2D vol surface, knock-in barriers, American MC/FD, proper SABR optimizer.

## License

MIT
