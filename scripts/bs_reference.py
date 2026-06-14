#!/usr/bin/env python3

import math
import sys


def norm_cdf(x):
    return 0.5 * math.erfc(-x / math.sqrt(2.0))


def black_scholes(option_type, S, K, r, sigma, T):
    sqrt_t = math.sqrt(T)
    d1 = (math.log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt_t)
    d2 = d1 - sigma * sqrt_t
    discount = math.exp(-r * T)

    if option_type == "call":
        return S * norm_cdf(d1) - K * discount * norm_cdf(d2)
    return K * discount * norm_cdf(-d2) - S * norm_cdf(-d1)


if __name__ == "__main__":
    S = float(sys.argv[1]) if len(sys.argv) > 1 else 100.0
    K = float(sys.argv[2]) if len(sys.argv) > 2 else 100.0
    r = float(sys.argv[3]) if len(sys.argv) > 3 else 0.05
    sigma = float(sys.argv[4]) if len(sys.argv) > 4 else 0.20
    T = float(sys.argv[5]) if len(sys.argv) > 5 else 1.0
    opt = sys.argv[6] if len(sys.argv) > 6 else "call"

    print(black_scholes(opt, S, K, r, sigma, T))
