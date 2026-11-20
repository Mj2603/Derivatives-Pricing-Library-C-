// Prints a Hagan (2002) SABR smile, then calibrates (alpha, rho, nu) back from it.
//
// Usage: sabr_smile [forward maturity alpha beta rho nu]
// Defaults: 100 1.0 2.0 0.5 -0.3 0.4
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "dpl/calibration/SABRCalibrator.hpp"

int main(int argc, char* argv[]) {
    double forward = 100.0;
    double maturity = 1.0;
    dpl::SABRParams params{2.0, 0.5, -0.3, 0.4};

    if (argc >= 7) {
        forward = std::stod(argv[1]);
        maturity = std::stod(argv[2]);
        params = {std::stod(argv[3]), std::stod(argv[4]), std::stod(argv[5]), std::stod(argv[6])};
    } else if (argc != 1) {
        std::cerr << "usage: " << argv[0] << " [forward maturity alpha beta rho nu]\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "SABR (Hagan 2002 lognormal)  F=" << forward << " T=" << maturity
              << " alpha=" << params.alpha << " beta=" << params.beta << " rho=" << params.rho
              << " nu=" << params.nu << "\n\n";
    std::cout << "  K/F      strike      implied_vol\n";

    std::vector<dpl::SABRMarketQuote> quotes;
    for (double m = 0.6; m <= 1.4001; m += 0.1) {
        const double k = forward * m;
        const double vol = dpl::sabr_implied_vol(params, forward, k, maturity);
        quotes.push_back({k, forward, maturity, vol});
        std::cout << "  " << std::setw(4) << std::setprecision(2) << m << std::setprecision(6)
                  << "  " << std::setw(12) << k << "  " << std::setw(12) << vol << "\n";
    }

    const dpl::SABRCalibrationResult fit = dpl::calibrate_sabr(quotes, params.beta);
    std::cout << "\nCalibrated from the " << quotes.size() << " quotes above (beta fixed at "
              << params.beta << "):\n";
    std::cout << "  alpha=" << fit.params.alpha << " rho=" << fit.params.rho
              << " nu=" << fit.params.nu << "\n";
    std::cout << std::scientific << std::setprecision(3) << "  rmse=" << fit.rmse
              << "  iterations=" << fit.iterations
              << "  converged=" << (fit.converged ? "yes" : "no") << "\n";
    return 0;
}
