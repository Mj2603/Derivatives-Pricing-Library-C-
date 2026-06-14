#pragma once

#include "dpl/core/Option.hpp"
#include "dpl/core/Types.hpp"

namespace dpl {

class MarketData;

class PricingEngine {
public:
    virtual ~PricingEngine() = default;

    virtual PricingResult price(const Option& option, const MarketData& market) const = 0;
};

}  // namespace dpl
