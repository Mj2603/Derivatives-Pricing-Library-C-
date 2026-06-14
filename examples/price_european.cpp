#include <iomanip>
#include <iostream>

#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

int main(int argc, char* argv[]) {
    double spot = 100.0;
    double strike = 100.0;
    double rate = 0.05;
    double vol = 0.20;
    double maturity = 1.0;
    bool is_call = true;

    if (argc >= 7) {
        spot = std::stod(argv[1]);
        strike = std::stod(argv[2]);
        rate = std::stod(argv[3]);
        vol = std::stod(argv[4]);
        maturity = std::stod(argv[5]);
        is_call = std::stoi(argv[6]) != 0;
    }

    const dpl::OptionType type = is_call ? dpl::OptionType::Call : dpl::OptionType::Put;
    const dpl::EuropeanOption option(type, strike, maturity);
    const dpl::MarketData market(spot, rate, vol);

    dpl::AnalyticalEngine engine;
    const dpl::PricingResult result = engine.price(option, market);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Price: " << result.price << "\n";
    std::cout << "Delta: " << result.greeks.delta << "\n";
    std::cout << "Gamma: " << result.greeks.gamma << "\n";
    std::cout << "Vega:  " << result.greeks.vega << "\n";
    std::cout << "Theta: " << result.greeks.theta << "\n";
    std::cout << "Rho:   " << result.greeks.rho << "\n";

    return 0;
}
