"""Independent reference for the Hagan et al. (2002) lognormal SABR implied volatility.

Written separately from the C++ implementation, using only the Python standard
library. Two independent evaluations of x(z) are used:

  * the closed form  x(z) = ln[(sqrt(1 - 2 rho z + z^2) + z - rho) / (1 - rho)]
  * the integral     x(z) = int_0^z dt / sqrt(1 - 2 rho t + t^2)
    evaluated with composite Simpson's rule,

and the script checks that they agree before printing reference values. The
F == K case uses Hagan's separate ATM formula (z / x(z) -> 1).

Reference: P. Hagan, D. Kumar, A. Lesniewski, D. Woodward,
"Managing Smile Risk", Wilmott Magazine (2002), eq. (2.17a)-(2.17b).

Usage:
    python3 scripts/sabr_reference.py            # print the table used by the C++ tests
    python3 scripts/sabr_reference.py F K T alpha beta rho nu
"""

import math
import sys


def x_closed_form(z, rho):
    return math.log((math.sqrt(1.0 - 2.0 * rho * z + z * z) + z - rho) / (1.0 - rho))


def x_integral(z, rho, n=20000):
    # Simpson's rule for int_0^z (1 - 2 rho t + t^2)^(-1/2) dt (n must be even).
    h = z / n
    f = lambda t: 1.0 / math.sqrt(1.0 - 2.0 * rho * t + t * t)
    total = f(0.0) + f(z)
    for i in range(1, n):
        total += (4.0 if i % 2 else 2.0) * f(i * h)
    return total * h / 3.0


def hagan_lognormal_vol(F, K, T, alpha, beta, rho, nu, x_func=x_closed_form):
    omb = 1.0 - beta
    if F == K:
        f_pow = F ** omb
        correction = (omb * omb / 24.0 * alpha * alpha / (F ** (2.0 * omb))
                      + 0.25 * rho * beta * nu * alpha / f_pow
                      + (2.0 - 3.0 * rho * rho) / 24.0 * nu * nu)
        return alpha / f_pow * (1.0 + correction * T)

    log_fk = math.log(F / K)
    fk_pow = (F * K) ** (omb / 2.0)
    denom = fk_pow * (1.0 + omb ** 2 / 24.0 * log_fk ** 2 + omb ** 4 / 1920.0 * log_fk ** 4)
    z = nu / alpha * fk_pow * log_fk
    z_over_x = z / x_func(z, rho)
    correction = (omb * omb / 24.0 * alpha * alpha / (fk_pow * fk_pow)
                  + 0.25 * rho * beta * nu * alpha / fk_pow
                  + (2.0 - 3.0 * rho * rho) / 24.0 * nu * nu)
    return alpha / denom * z_over_x * (1.0 + correction * T)


# (F, K, T, alpha, beta, rho, nu) cases mirrored in tests/test_sabr.cpp
CASES = [
    (100.0, 70.0, 1.0, 2.0, 0.5, -0.3, 0.4),
    (100.0, 90.0, 1.0, 2.0, 0.5, -0.3, 0.4),
    (100.0, 100.0, 1.0, 2.0, 0.5, -0.3, 0.4),
    (100.0, 110.0, 1.0, 2.0, 0.5, -0.3, 0.4),
    (100.0, 130.0, 1.0, 2.0, 0.5, -0.3, 0.4),
    (0.03, 0.02, 5.0, 0.035, 0.5, -0.25, 0.6),
    (0.03, 0.045, 5.0, 0.035, 0.5, -0.25, 0.6),
    (100.0, 80.0, 0.5, 0.25, 1.0, 0.2, 0.8),
    (100.0, 125.0, 0.5, 0.25, 1.0, 0.2, 0.8),
    (50.0, 40.0, 2.0, 3.0, 0.0, 0.0, 0.3),
    (50.0, 65.0, 2.0, 3.0, 0.0, 0.0, 0.3),
]


def main():
    if len(sys.argv) == 8:
        args = [float(a) for a in sys.argv[1:]]
        print(f"{hagan_lognormal_vol(*args):.12f}")
        return

    print("   F        K       T     alpha   beta   rho    nu    hagan_vol")
    for case in CASES:
        vol = hagan_lognormal_vol(*case)
        if case[0] != case[1]:
            vol_int = hagan_lognormal_vol(*case, x_func=x_integral)
            assert abs(vol - vol_int) < 1e-11, (case, vol, vol_int)
        print("{:7g} {:8g} {:5g} {:7g} {:5g} {:6g} {:5g}  {:.12f}".format(*case, vol))

    # ATM continuity: the general formula approaches the ATM formula as K -> F.
    F, T, alpha, beta, rho, nu = 100.0, 1.0, 2.0, 0.5, -0.3, 0.4
    atm = hagan_lognormal_vol(F, F, T, alpha, beta, rho, nu)
    for dk in (1e-2, 1e-3, 1e-4):
        near = hagan_lognormal_vol(F, F + dk, T, alpha, beta, rho, nu)
        print(f"ATM continuity dK={dk:g}: |vol(F+dK) - vol(F)| = {abs(near - atm):.3e}")


if __name__ == "__main__":
    main()
