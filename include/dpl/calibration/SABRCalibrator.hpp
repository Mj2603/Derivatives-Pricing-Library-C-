#pragma once

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
    double market_vol;
};

SABRParams calibrate_sabr(const SABRMarketQuote& quote, double initial_alpha = 0.1,
                          double beta = 0.5);

double sabr_implied_vol(const SABRParams& params, double forward, double strike,
                        double maturity);

}  // namespace dpl
