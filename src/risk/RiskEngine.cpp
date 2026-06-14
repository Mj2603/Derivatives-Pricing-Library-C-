#include "dpl/risk/RiskEngine.hpp"

#include "dpl/engines/AnalyticalEngine.hpp"

namespace dpl {

void RiskEngine::add_position(const EuropeanOption& option, const MarketData& market,
                              double quantity) {
    positions_.push_back({option, market, quantity});
}

PortfolioRisk RiskEngine::compute_risk() const {
    PortfolioRisk risk{0.0, 0.0, 0.0};

    for (const auto& pos : positions_) {
        const PricingResult result = engine_.price(pos.option, pos.market);
        risk.delta += pos.quantity * result.greeks.delta;
        risk.gamma += pos.quantity * result.greeks.gamma;
        risk.vega += pos.quantity * result.greeks.vega;
    }

    return risk;
}

std::size_t RiskEngine::num_positions() const { return positions_.size(); }

}  // namespace dpl
