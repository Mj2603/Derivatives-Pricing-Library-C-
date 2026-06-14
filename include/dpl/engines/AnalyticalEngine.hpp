#pragma once

#include "dpl/engines/PricingEngine.hpp"

namespace dpl {

class AnalyticalEngine : public PricingEngine {
public:
    PricingResult price(const Option& option, const MarketData& market) const override;
};

}  // namespace dpl
