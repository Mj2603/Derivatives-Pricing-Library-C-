#pragma once

#include "dpl/engines/PricingEngine.hpp"

namespace dpl {

enum class MCVarianceReduction { None, Antithetic, ControlVariate };

class MonteCarloEngine : public PricingEngine {
public:
    MonteCarloEngine(int num_paths, int num_steps, unsigned seed = 42);

    void set_variance_reduction(MCVarianceReduction method);
    PricingResult price(const Option& option, const MarketData& market) const override;

    int num_paths() const;

private:
    int num_paths_;
    int num_steps_;
    unsigned seed_;
    MCVarianceReduction variance_reduction_;
};

}  // namespace dpl
