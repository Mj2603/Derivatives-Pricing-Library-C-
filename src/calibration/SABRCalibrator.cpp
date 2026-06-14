#include "dpl/calibration/SABRCalibrator.hpp"

#include <algorithm>
#include <cmath>

namespace dpl {

namespace {

double sabr_z(double strike, double forward, double alpha, double beta, double rho,
              double nu) {
    const double eps = 1e-8;
    if (std::abs(strike - forward) < eps) {
        return 0.0;
    }
    const double FK = forward * strike;
    const double FK_beta = std::pow(FK, (1.0 - beta) / 2.0);
    const double log_FK = std::log(forward / strike);
    const double term1 = nu / alpha * FK_beta * log_FK;
    const double term2 = std::sqrt(1.0 - 2.0 * rho * term1 + term1 * term1);
    return term1 / (1.0 - rho + term2);
}

double sabr_x(double z, double rho) {
    const double eps = 1e-8;
    if (std::abs(z) < eps) {
        return 1.0;
    }
    const double term = std::sqrt(1.0 - 2.0 * rho * z + z * z) + z - rho;
    return z / term;
}

}  // namespace

double sabr_implied_vol(const SABRParams& params, double forward, double strike,
                        double maturity) {
    const double eps = 1e-8;
    if (maturity <= eps) {
        return params.alpha;
    }

    const double FK = forward * strike;
    const double FK_beta = std::pow(FK, (1.0 - params.beta) / 2.0);
    const double A = params.alpha / FK_beta;
    const double B = 1.0
                     + ((1.0 - params.beta) * (1.0 - params.beta) / 24.0 * params.alpha * params.alpha
                        / std::pow(FK, 1.0 - params.beta)
                        + 0.25 * params.rho * params.beta * params.nu * params.alpha
                              / std::pow(FK, (1.0 - params.beta) / 2.0)
                        + (2.0 - 3.0 * params.rho * params.rho) / 24.0 * params.nu * params.nu)
                           * maturity;

    const double z = sabr_z(strike, forward, params.alpha, params.beta, params.rho,
                            params.nu);
    const double xz = sabr_x(z, params.rho);

    return A * B * xz;
}

SABRParams calibrate_sabr(const SABRMarketQuote& quote, double initial_alpha,
                          double beta) {
    SABRParams params{initial_alpha, beta, -0.3, 0.4};

    // Grid search over rho and nu; alpha updated by vol error at each step
    double best_error = 1e10;
    SABRParams best = params;

    for (double rho = -0.9; rho <= 0.9; rho += 0.3) {
        for (double nu = 0.1; nu <= 1.0; nu += 0.3) {
            double alpha = initial_alpha;
            for (int iter = 0; iter < 20; ++iter) {
                params = {alpha, beta, rho, nu};
                const double model_vol = sabr_implied_vol(params, quote.forward,
                                                          quote.strike, quote.maturity);
                const double error = model_vol - quote.market_vol;
                if (std::abs(error) < best_error) {
                    best_error = std::abs(error);
                    best = params;
                }
                alpha -= 0.5 * error;
                alpha = std::max(alpha, 1e-4);
            }
        }
    }

    return best;
}

}  // namespace dpl
