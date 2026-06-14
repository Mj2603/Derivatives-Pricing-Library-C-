#pragma once

#include "dpl/engines/PricingEngine.hpp"

namespace dpl {

enum class FDScheme { Explicit, Implicit, CrankNicolson };

class FiniteDifferenceEngine : public PricingEngine {
public:
    FiniteDifferenceEngine(FDScheme scheme, int space_steps, int time_steps);

    PricingResult price(const Option& option, const MarketData& market) const override;

private:
    FDScheme scheme_;
    int space_steps_;
    int time_steps_;
};

}  // namespace dpl
