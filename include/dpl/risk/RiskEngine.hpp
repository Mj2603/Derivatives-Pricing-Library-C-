#pragma once

#include "dpl/core/Types.hpp"
#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <vector>

namespace dpl {

struct PortfolioPosition {
    EuropeanOption option;
    MarketData market;
    double quantity;
};

struct PortfolioRisk {
    double delta;
    double gamma;
    double vega;
};

class RiskEngine {
public:
    void add_position(const EuropeanOption& option, const MarketData& market,
                      double quantity);
    PortfolioRisk compute_risk() const;
    std::size_t num_positions() const;

private:
    std::vector<PortfolioPosition> positions_;
    AnalyticalEngine engine_;
};

}  // namespace dpl
