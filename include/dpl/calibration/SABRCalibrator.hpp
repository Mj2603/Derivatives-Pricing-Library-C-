#pragma once

#include <vector>

namespace dpl {

struct SABRParams {
    double alpha;
    double beta;
    double rho;
    double nu;
};

struct SABRMarketQuote {
    double strike;
    double forward;
    double maturity;
    double market_vol;  // Black (lognormal) implied volatility
};

// Hagan, Kumar, Lesniewski & Woodward (2002) lognormal implied-volatility
// approximation. Requires forward > 0, strike > 0, maturity >= 0, alpha > 0,
// 0 <= beta <= 1, -1 < rho < 1 and nu >= 0; throws std::invalid_argument
// otherwise. Near F == K the ratio z / x(z) is evaluated with its Taylor series,
// so the smile is continuous through ATM.
double sabr_implied_vol(const SABRParams& params, double forward, double strike,
                        double maturity);

struct SABRCalibrationOptions {
    int max_iterations = 5000;  // Nelder-Mead iterations per run
    int restarts = 2;           // extra Nelder-Mead runs restarted from the best point
    double tolerance = 1e-14;   // stop when the simplex objective spread is below this
    double rho_bound = 0.999;   // enforced: |rho| < rho_bound
    double nu_max = 5.0;        // enforced: 0 < nu < nu_max
};

struct SABRCalibrationResult {
    SABRParams params;
    double rmse;     // root-mean-square implied-vol error over the quotes
    int iterations;  // total Nelder-Mead iterations across all runs
    bool converged;  // final run reached the tolerance before max_iterations
};

// Least-squares fit of (alpha, rho, nu) with beta held fixed by the caller,
// minimising sum_i (sabr_implied_vol(K_i) - market_vol_i)^2 with an in-repo
// Nelder-Mead simplex. Bounds are enforced by reparameterisation
// (alpha = exp(u), rho = rho_bound * tanh(v), nu = nu_max * logistic(w)).
// The starting alpha is solved from the quote closest to ATM for a small grid
// of (rho, nu) seeds. Each quote carries its own forward and maturity; in
// practice a smile is one expiry. Requires at least 3 quotes.
SABRCalibrationResult calibrate_sabr(const std::vector<SABRMarketQuote>& quotes,
                                     double beta,
                                     const SABRCalibrationOptions& options = {});

}  // namespace dpl
