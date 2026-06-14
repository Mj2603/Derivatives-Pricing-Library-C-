#include <gtest/gtest.h>

#include "dpl/calibration/ImpliedVolSolver.hpp"
#include "dpl/calibration/SABRCalibrator.hpp"
#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/instruments/EuropeanOption.hpp"
#include "dpl/market/VolSurface.hpp"

#include <cmath>
#include <vector>

TEST(VolSurface, CubicSpline_interpolates_known_points) {
    const std::vector<double> strikes = {80.0, 90.0, 100.0, 110.0, 120.0};
    const std::vector<double> vols = {0.28, 0.24, 0.20, 0.22, 0.26};

    dpl::VolSurface surface(strikes, vols);

    EXPECT_NEAR(surface.volatility(100.0), 0.20, 1e-6);
    EXPECT_NEAR(surface.volatility(90.0), 0.24, 1e-6);
    EXPECT_GT(surface.volatility(95.0), 0.20);
    EXPECT_LT(surface.volatility(95.0), 0.24);
}

TEST(VolSurface, Strike_dependent_vol_affects_analytical_price) {
    const std::vector<double> strikes = {90.0, 100.0, 110.0};
    const std::vector<double> vols = {0.24, 0.20, 0.22};
    const dpl::VolSurface surface(strikes, vols);

    const dpl::EuropeanOption atm(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData flat(100.0, 0.05, 0.20);
    const dpl::MarketData smile(100.0, 0.05, 0.20, surface);

    dpl::AnalyticalEngine engine;
    const double flat_price = engine.price(atm, flat).price;
    const double smile_price = engine.price(atm, smile).price;

    EXPECT_NEAR(flat_price, smile_price, 1e-6);

    const dpl::EuropeanOption otm(dpl::OptionType::Call, 110.0, 1.0);
    const double otm_flat = engine.price(otm, flat).price;
    const double otm_smile = engine.price(otm, smile).price;

    EXPECT_NE(otm_flat, otm_smile);
}

TEST(ImpliedVol, Recovers_input_volatility) {
    const double true_vol = 0.25;
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, true_vol);

    dpl::AnalyticalEngine engine;
    const double market_price = engine.price(option, market).price;

    const double recovered = dpl::implied_volatility(dpl::OptionType::Call, market_price,
                                                     market, 100.0, 1.0);

    EXPECT_NEAR(recovered, true_vol, 1e-4);
}

TEST(SABR, Calibration_reduces_model_error) {
    dpl::SABRMarketQuote quote{100.0, 100.0, 1.0, 0.22};

    const dpl::SABRParams initial{0.1, 0.5, 0.0, 0.3};
    const double initial_vol = dpl::sabr_implied_vol(initial, quote.forward,
                                                     quote.strike, quote.maturity);
    const double initial_error = std::abs(initial_vol - quote.market_vol);

    const dpl::SABRParams calibrated = dpl::calibrate_sabr(quote);
    const double calibrated_vol = dpl::sabr_implied_vol(calibrated, quote.forward,
                                                        quote.strike, quote.maturity);
    const double calibrated_error = std::abs(calibrated_vol - quote.market_vol);

    EXPECT_LT(calibrated_error, initial_error);
}
