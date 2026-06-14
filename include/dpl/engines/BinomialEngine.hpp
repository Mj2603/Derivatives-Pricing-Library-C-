#pragma once

#include "dpl/engines/PricingEngine.hpp"

namespace dpl {

class BinomialEngine : public PricingEngine {
public:
    explicit BinomialEngine(int steps);

    PricingResult price(const Option& option, const MarketData& market) const override;

private:
    int steps_;
};

}  // namespace dpl
